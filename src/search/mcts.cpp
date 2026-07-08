#include "include/search/mcts.h"
#include <algorithm>
#include <iostream>

namespace fenrir
{
	namespace
	{
		uint32_t move_index(const Move &m)
		{
			// Explicitly cast to int for coordinates calculation
			int from_sq = static_cast<int>(m.get_from_square());
			int to_sq = static_cast<int>(m.get_to_square());

			int from_file = from_sq % 8;
			int from_rank = from_sq / 8;
			int to_file = to_sq % 8;
			int to_rank = to_sq / 8;

			int dx = to_file - from_file;
			int dy = to_rank - from_rank;

			int channel = 0;

			char promo = static_cast<char>(std::tolower(static_cast<unsigned char>(m.get_promotion_piece())));

			// 1. Handle Underpromotions (Knight, Bishop, Rook)
			if (m.is_promotion() && promo != 'q' && promo != '\0')
			{
				int promo_dir = dx + 1; // -1 -> 0 (left), 0 -> 1 (straight), 1 -> 2 (right)
				int promo_type = 0;
				if (promo == 'b')
					promo_type = 1;
				else if (promo == 'r')
					promo_type = 2;

				channel = 64 + (promo_dir * 3) + promo_type;
			}
			// 2. Handle Knight Moves
			else if (std::abs(dx) * std::abs(dy) == 2)
			{
				std::pair<int, int> knight_lookups[8] = {
					{1, 2}, {2, 1}, {2, -1}, {1, -2}, {-1, -2}, {-2, -1}, {-2, 1}, {-1, 2}};
				for (int i = 0; i < 8; ++i)
				{
					if (dx == knight_lookups[i].first && dy == knight_lookups[i].second)
					{
						channel = 56 + i;
						break;
					}
				}
			}
			// 3. Handle Regular Moves (Queen moves / King moves / Pawn steps / Queen Promotions)
			else
			{
				int step_x = (dx == 0) ? 0 : (dx > 0 ? 1 : -1);
				int step_y = (dy == 0) ? 0 : (dy > 0 ? 1 : -1);
				int distance = std::max(std::abs(dx), std::abs(dy));

				std::pair<int, int> dirs[8] = {
					{0, 1}, {1, 1}, {1, 0}, {1, -1}, {0, -1}, {-1, -1}, {-1, 0}, {-1, 1} // N, NE, E, SE, S, SW, W, NW
				};

				int dir_idx = 0;
				for (int i = 0; i < 8; ++i)
				{
					if (step_x == dirs[i].first && step_y == dirs[i].second)
					{
						dir_idx = i;
						break;
					}
				}
				channel = (dir_idx * 7) + (distance - 1);
			}

			return static_cast<uint32_t>((from_sq * 73) + channel);
		}
	}

	MCTSNode::MCTSNode(MCTSNode *parent_node, const Move &m, uint8_t color)
		: parent(parent_node), move(m), color_to_move(color),
		  visits(0), win_score(0.0), virtual_loss(0), prior(0.0), is_expanded(false)
	{
	}

	MCTSNode::~MCTSNode() = default;

	void MCTSNode::expand(Engine &engine, const std::vector<double> &policy)
	{
		std::lock_guard<std::mutex> lock(expand_mutex);
		if (is_expanded.load())
			return;

		std::vector<Move> moves = engine.generate_all_moves();
		for (size_t i = 0; i < moves.size(); ++i)
		{
			uint8_t next_color = 1 - color_to_move;
			children.push_back(std::make_unique<MCTSNode>(this, moves[i], next_color));

			if (policy.empty())
			{
				children.back()->prior = 1.0 / static_cast<double>(moves.size());
			}
			else
			{
				uint32_t idx = move_index(moves[i]);
				children.back()->prior = (idx < policy.size()) ? policy[idx] : 0.0;
			}
		}
		is_expanded.store(true);
	}

	MCTSNode *MCTSNode::select_child()
	{
		MCTSNode *best_child = nullptr;
		double best_value = -1e9;

		int parent_visits = visits.load() + virtual_loss.load();

		for (auto &child : children)
		{
			double puct = child->puct_value(parent_visits);
			if (puct > best_value)
			{
				best_value = puct;
				best_child = child.get();
			}
		}
		return best_child;
	}

	double MCTSNode::puct_value(int total_visits) const
	{
		int v = visits.load() + virtual_loss.load();
		double q = (v == 0) ? 0.0 : (1.0 - (win_score.load() / v));
		double c = 1.414;
		double u = c * prior * std::sqrt(total_visits) / (1 + v);
		return q + u;
	}

	int MCTSNode::get_max_depth() const
	{
		if (!is_expanded.load() || children.empty())
			return 0;
		int max_d = 0;
		for (const auto &child : children)
		{
			if (child->visits.load() > 0)
			{
				max_d = std::max(max_d, child->get_max_depth());
			}
		}
		return 1 + max_d;
	}

	void MCTSNode::add_dirichlet_noise(double epsilon, double alpha)
	{
		if (children.empty())
			return;
		std::mt19937 rng(std::random_device{}());
		std::gamma_distribution<double> gamma(alpha, 1.0);

		double sum = 0.0;
		std::vector<double> noise;
		noise.reserve(children.size());
		for (size_t i = 0; i < children.size(); ++i)
		{
			double n = gamma(rng);
			noise.push_back(n);
			sum += n;
		}

		for (size_t i = 0; i < children.size(); ++i)
		{
			children[i]->prior = (1.0 - epsilon) * children[i]->prior + epsilon * (noise[i] / sum);
		}
	}

	void MCTSNode::backpropagate(double result)
	{
		virtual_loss--;
		visits++;
		double current = win_score.load();
		while (!win_score.compare_exchange_weak(current, current + result))
			;
		if (parent)
			parent->backpropagate(1.0 - result);
	}

	MCTSSearch::MCTSSearch(NNEvaluator *eval, int threads)
		: evaluator(eval), num_threads(threads) {}

	MCTSSearch::~MCTSSearch() = default;

	Move MCTSSearch::find_best_move(Engine &engine, int time_limit_ms, int max_simulations)
	{
		uint8_t root_color = engine.get_board_view().get_color();
		Move empty_move(0, 0);
		auto root = std::make_unique<MCTSNode>(nullptr, empty_move, root_color);

		if (evaluator)
		{
			auto future = evaluator->request_evaluation(engine.get_board_view());
			NNResult res = future.get();
			root->expand(engine, res.policy);
		}
		else
		{
			root->expand(engine);
		}

		if (root->children.empty())
		{
			return empty_move;
		}

		std::vector<std::thread> workers;
		int sims_per_thread = -1;
		bool use_time = (time_limit_ms > 0);

		if (!use_time)
		{
			sims_per_thread = num_threads > 0 ? max_simulations / num_threads : max_simulations;
			if (sims_per_thread <= 0)
				sims_per_thread = 1;
		}

		auto end_time = std::chrono::steady_clock::now() + std::chrono::milliseconds(std::max(1, time_limit_ms));

		for (int i = 0; i < num_threads; ++i)
		{
			workers.emplace_back(&MCTSSearch::search_worker, this, std::make_unique<Engine>(engine.get_fen()), root.get(), sims_per_thread, end_time, use_time);
		}

		for (auto &w : workers)
		{
			w.join();
		}

		Move best_move(0, 0);
		int max_visits = -1;
		double best_score = 0.5;
		for (auto &child : root->children)
		{
			if (child->visits.load() > max_visits)
			{
				max_visits = child->visits.load();
				best_move = child->move;
				best_score = child->win_score.load() / std::max(1, child->visits.load());
			}
		}

		int total_nodes = root->visits.load();
		int max_depth = root->get_max_depth();
		int score_cp = static_cast<int>((best_score - 0.5) * 200); // Convert win rate [0, 1] to centipawns
		std::cout << "info depth " << max_depth << " nodes " << total_nodes << " score cp " << score_cp << " pv " << best_move.to_uci_notation() << std::endl;

		return best_move;
	}

	std::pair<Move, std::vector<std::pair<Move, double>>> MCTSSearch::find_best_move_with_policy(Engine &engine, int simulations, bool apply_noise)
	{
		uint8_t root_color = engine.get_board_view().get_color();
		Move empty_move(0, 0);
		auto root = std::make_unique<MCTSNode>(nullptr, empty_move, root_color);

		if (evaluator)
		{
			try
			{
				auto future = evaluator->request_evaluation(engine.get_board_view());
				NNResult res = future.get();
				root->expand(engine, res.policy);
			}
			catch (const std::exception &e)
			{
				(void)e; // silence unreferenced variable warning
				std::cerr << "Root evaluation failed: " << e.what() << "\n";
				root->expand(engine);
			}
		}
		else
		{
			root->expand(engine);
		}

		if (apply_noise)
		{
			root->add_dirichlet_noise(0.25, 0.3); // 25% noise, alpha 0.3
		}

		if (root->children.empty())
		{
			return {empty_move, {}};
		}

		std::vector<std::thread> workers;
		int sims_per_thread = num_threads > 0 ? simulations / num_threads : simulations;
		if (sims_per_thread == 0)
			sims_per_thread = 1;

		for (int i = 0; i < num_threads; ++i)
		{
			try
			{
				workers.emplace_back(&MCTSSearch::search_worker, this, std::make_unique<Engine>(engine.get_fen()), root.get(), sims_per_thread, std::chrono::steady_clock::now(), false);
			}
			catch (const std::exception &e)
			{
				std::cerr << "Thread creation failed: " << e.what() << "\n";
				break; // Just proceed with the threads we successfully created
			}
		}

		if (workers.empty())
		{
			// Fallback to synchronous search on the main thread if OS is completely out of handles
			std::cerr << "Warning: Falling back to synchronous search!\n";
			search_worker(std::make_unique<Engine>(engine.get_fen()), root.get(), simulations, std::chrono::steady_clock::now(), false);
		}

		for (auto &w : workers)
		{
			if (w.joinable())
			{
				w.join();
			}
		}

		Move best_move(0, 0);
		int max_visits = -1;
		std::vector<std::pair<Move, double>> policy;
		double total_visits = root->visits.load();

		for (auto &child : root->children)
		{
			int v = child->visits.load();
			policy.push_back({child->move, static_cast<double>(v) / std::max(1.0, total_visits)});
			if (v > max_visits)
			{
				max_visits = v;
				best_move = child->move;
			}
		}
		return {best_move, policy};
	}

	void MCTSSearch::search_worker(std::unique_ptr<Engine> thread_engine_ptr, MCTSNode *root, int simulations, std::chrono::steady_clock::time_point end_time, bool use_time)
	{
		Engine &thread_engine = *thread_engine_ptr;
		std::mt19937 local_rng(std::random_device{}());
		int sim_count = 0;

		while (true)
		{
			if (use_time)
			{
				// Check time periodically to avoid massive std::chrono syscall overhead
				if ((sim_count & 15) == 0 && std::chrono::steady_clock::now() >= end_time)
				{
					break;
				}
			}
			else
			{
				if (sim_count >= simulations)
				{
					break;
				}
			}
			sim_count++;
			MCTSNode *node = root;

			while (node->is_expanded.load() && !node->children.empty())
			{
				node->virtual_loss++;
				node = node->select_child();
				thread_engine.make_move_fast(node->move);
			}
			node->virtual_loss++;

			double result = 0.0;
			if (thread_engine.is_checkmate())
			{
				result = 0.0;
			}
			else if (thread_engine.is_stalemate() || thread_engine.is_draw())
			{
				result = 0.5;
			}
			else
			{
				if (evaluator)
				{
					try
					{
						auto future = evaluator->request_evaluation(thread_engine.get_board_view());
						NNResult res = future.get();
						node->expand(thread_engine, res.policy);
						result = res.value;
					}
					catch (const std::exception &e)
					{
						(void)e; // silence unreferenced variable warning
						node->expand(thread_engine);
						result = simulate(thread_engine);
					}
				}
				else
				{
					node->expand(thread_engine);
					result = simulate(thread_engine);
				}
			}

			node->backpropagate(result);

			MCTSNode *curr = node;
			while (curr != root)
			{
				thread_engine.undo_move();
				curr = curr->parent;
			}
		}
	}

	double MCTSSearch::simulate(Engine &engine)
	{
		int moves_played = 0;
		uint8_t start_color = engine.get_board_view().get_color();
		std::mt19937 local_rng(std::random_device{}());

		while (!engine.is_checkmate() && !engine.is_stalemate() && !engine.is_draw() && moves_played < 100)
		{
			std::vector<Move> legal_moves = engine.generate_all_moves();
			if (legal_moves.empty())
				break;

			std::uniform_int_distribution<size_t> dist(0, legal_moves.size() - 1);
			engine.make_move_fast(legal_moves[dist(local_rng)]);
			moves_played++;
		}

		double result = 0.5;
		if (engine.is_checkmate())
		{
			if (engine.get_board_view().get_color() == start_color)
			{
				result = 0.0;
			}
			else
			{
				result = 1.0;
			}
		}

		for (int i = 0; i < moves_played; ++i)
		{
			engine.undo_move();
		}

		return result;
	}
}
