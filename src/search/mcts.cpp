#include "include/search/mcts.h"
#include <algorithm>
#include <iostream>
#include <random>

namespace fenrir
{
	namespace
	{
		uint32_t move_index(const Move &m)
		{
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

			if (m.is_promotion() && promo != 'q' && promo != '\0')
			{
				int promo_dir = dx + 1;
				int promo_type = 0;
				if (promo == 'b')
					promo_type = 1;
				else if (promo == 'r')
					promo_type = 2;

				channel = 64 + (promo_dir * 3) + promo_type;
			}
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
			else
			{
				int step_x = (dx == 0) ? 0 : (dx > 0 ? 1 : -1);
				int step_y = (dy == 0) ? 0 : (dy > 0 ? 1 : -1);
				int distance = std::max(std::abs(dx), std::abs(dy));

				int dir_idx = 0;
				if (step_x == 0 && step_y == 1)
					dir_idx = 0;
				else if (step_x == 1 && step_y == 1)
					dir_idx = 1;
				else if (step_x == 1 && step_y == 0)
					dir_idx = 2;
				else if (step_x == 1 && step_y == -1)
					dir_idx = 3;
				else if (step_x == 0 && step_y == -1)
					dir_idx = 4;
				else if (step_x == -1 && step_y == -1)
					dir_idx = 5;
				else if (step_x == -1 && step_y == 0)
					dir_idx = 6;
				else if (step_x == -1 && step_y == 1)
					dir_idx = 7;

				channel = (dir_idx * 7) + (distance - 1);
			}

			return static_cast<uint32_t>((from_sq * 73) + channel);
		}

		// RAII helper to ensure virtual loss is perfectly decremented regardless of exceptions
		struct VirtualLossGuard
		{
			MCTSNode *node;
			VirtualLossGuard(MCTSNode *n) : node(n) { node->virtual_loss++; }
			~VirtualLossGuard() { node->virtual_loss--; }
		};
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
		children.reserve(moves.size());

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

		// 1. If unvisited, inherit parent's neutral baseline, don't reward it with a fake 0.0 draw
		double q = 0.0;
		if (v > 0)
		{
			q = win_score.load() / v;
		}
		else if (parent)
		{
			// Fallback or tiny penalty to prevent massive horizontal unvisited node tracking
			q = 0.0;
		}

		// 2. Standard AlphaZero CPUCT dynamic exploration constant
		// At 1.414, it might widen too fast. True AlphaZero scales c dynamically:
		double c_init = 1.25;
		double c_base = 19652.0;
		double c = std::log((1.0 + total_visits + c_base) / c_base) + c_init;

		// 3. Calculate regular upper confidence bound component
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

		double original_prior_sum = 0.0;
		for (const auto &child : children)
		{
			original_prior_sum += child->prior;
		}
		if (original_prior_sum <= 0.0)
			original_prior_sum = 1.0;

		double noise_sum = 0.0;
		std::vector<double> noise;
		noise.reserve(children.size());
		for (size_t i = 0; i < children.size(); ++i)
		{
			double n = gamma(rng);
			noise.push_back(n);
			noise_sum += n;
		}
		if (noise_sum <= 0.0)
			noise_sum = 1.0;

		for (size_t i = 0; i < children.size(); ++i)
		{
			double normalized_original = children[i]->prior / original_prior_sum;
			double normalized_noise = noise[i] / noise_sum;
			children[i]->prior = (1.0 - epsilon) * normalized_original + epsilon * normalized_noise;
		}
	}

	void MCTSNode::backpropagate(double result)
	{
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
			return empty_move;

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
		int score_cp = static_cast<int>((best_score - 0.5) * 200);
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
				(void)e;
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
			root->add_dirichlet_noise(0.25, 0.3);
		}

		if (root->children.empty())
			return {empty_move, {}};

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
				break;
			}
		}

		if (workers.empty())
		{
			std::cerr << "Warning: Falling back to synchronous search!\n";
			search_worker(std::make_unique<Engine>(engine.get_fen()), root.get(), simulations, std::chrono::steady_clock::now(), false);
		}

		for (auto &w : workers)
		{
			if (w.joinable())
				w.join();
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
		int sim_count = 0;

		const size_t local_pipeline_target = 64;

		struct PipelineItem
		{
			MCTSNode *node;
			std::vector<VirtualLossGuard> path_guards;
			std::future<NNResult> eval_future;
			bool is_terminal;
			double terminal_result;
		};

		while (true)
		{
			if (use_time)
			{
				if ((sim_count & 15) == 0 && std::chrono::steady_clock::now() >= end_time)
					break;
			}
			else
			{
				if (sim_count >= simulations)
					break;
			}

			std::vector<PipelineItem> pipeline;
			pipeline.reserve(local_pipeline_target);

			for (size_t p = 0; p < local_pipeline_target; ++p)
			{
				sim_count++;
				MCTSNode *node = root;
				std::vector<VirtualLossGuard> guards;

				while (node->is_expanded.load() && !node->children.empty())
				{
					guards.emplace_back(node);
					MCTSNode *next_node = node->select_child();

					if (!next_node)
						break;

					node = next_node;
					thread_engine.make_move_fast(node->move);
				}
				guards.emplace_back(node);

				PipelineItem item;
				item.node = node;
				item.path_guards = std::move(guards);
				item.is_terminal = false;
				item.terminal_result = 0.0;

				if (thread_engine.is_checkmate())
				{
					item.is_terminal = true;
					item.terminal_result = 0.0;
				}
				else if (thread_engine.is_stalemate() || thread_engine.is_draw())
				{
					item.is_terminal = true;
					item.terminal_result = 0.5;
				}
				else if (evaluator)
				{
					item.eval_future = evaluator->request_evaluation(thread_engine.get_board_view());
				}
				else
				{
					item.is_terminal = true;
					node->expand(thread_engine);
					item.terminal_result = simulate(thread_engine);
				}

				pipeline.push_back(std::move(item));

				MCTSNode *curr = node;
				while (curr != root)
				{
					thread_engine.undo_move();
					curr = curr->parent;
				}
			}

			for (auto &item : pipeline)
			{
				double final_result = 0.0;

				if (item.is_terminal)
				{
					final_result = item.terminal_result;
					if (!item.node->is_expanded.load())
					{
						// Handled terminal or rollout states
					}
				}
				else
				{
					try
					{
						NNResult res = item.eval_future.get();
						item.node->expand(thread_engine, res.policy);
						final_result = res.value;
					}
					catch (...)
					{
						item.node->expand(thread_engine);
						final_result = simulate(thread_engine);
					}
				}

				item.path_guards.clear();
				item.node->backpropagate(final_result);
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
			result = (engine.get_board_view().get_color() == start_color) ? 0.0 : 1.0;
		}

		for (int i = 0; i < moves_played; ++i)
		{
			engine.undo_move();
		}

		return result;
	}
}
