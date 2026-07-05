import ctypes
import os
import random

# Load the main Fenrir library (which now includes the C API wrapper)
lib_path = os.path.join(os.path.dirname(os.path.dirname(__file__)), "bin", "lib", "libfenrir.so")
try:
    lib = ctypes.CDLL(lib_path)
except OSError as e:
    print(f"Failed to load library: {e}")
    print("Make sure you built the project with CMake.")
    exit(1)

# Define function signatures
lib.fenrir_create.restype = ctypes.c_void_p
lib.fenrir_destroy.argtypes = [ctypes.c_void_p]

lib.fenrir_make_move.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_char_p]
lib.fenrir_make_move.restype = ctypes.c_bool

lib.fenrir_generate_all_moves.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_size_t]
lib.fenrir_generate_all_moves.restype = ctypes.c_int

lib.fenrir_get_fen.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_size_t]

lib.fenrir_print_board.argtypes = [ctypes.c_void_p]

class FenrirEngine:
    def __init__(self):
        self.engine = lib.fenrir_create()
        
    def __del__(self):
        if hasattr(self, 'engine') and self.engine:
            lib.fenrir_destroy(self.engine)
            
    def make_move(self, from_sq: str, to_sq: str) -> bool:
        return lib.fenrir_make_move(self.engine, from_sq.encode('utf-8'), to_sq.encode('utf-8'))
        
    def get_all_moves(self) -> list[str]:
        buffer = ctypes.create_string_buffer(8192)
        count = lib.fenrir_generate_all_moves(self.engine, buffer, 8192)
        moves_str = buffer.value.decode('utf-8')
        if not moves_str:
            return []
        return moves_str.split(',')
        
    def get_fen(self) -> str:
        buffer = ctypes.create_string_buffer(256)
        lib.fenrir_get_fen(self.engine, buffer, 256)
        return buffer.value.decode('utf-8')
        
    def print_board(self):
        lib.fenrir_print_board(self.engine)
        
    def get_turn(self) -> str:
        fen = self.get_fen()
        # FEN format: pieces turn castling en_passant halfmove fullmove
        parts = fen.split(' ')
        if len(parts) > 1:
            return "White" if parts[1] == 'w' else "Black"
        return "Unknown"

def main():
    print("=== Fenrir Strict Rule Enforcement Demo ===")
    engine = FenrirEngine()
    
    print(f"Initial FEN: {engine.get_fen()}")
    print(f"Turn: {engine.get_turn()}")
    
    # 1. Deliberately try to make an illegal / out-of-turn move
    print("\n[TEST] Attempting to move Black's pawn (e7 to e5) when it is White's turn:")
    success = engine.make_move("e7", "e5")
    if success:
        print("FAIL: The engine allowed an illegal move!")
    else:
        print("PASS: The engine firmly rejected the move and protected the game state.")
        
    print("\nBoard state remains unchanged:")
    engine.print_board()
    
    # 2. Play a few random legal moves
    print("\n[TEST] Playing 3 random legal moves to ensure normal play works:")
    for i in range(3):
        turn = engine.get_turn()
        moves = engine.get_all_moves()
        
        if not moves:
            print("No moves available. Game over?")
            break
            
        move = random.choice(moves)
        from_sq, to_sq = move[:2], move[2:4]
        
        print(f"\n{turn} plays legal move: {move} ({from_sq} -> {to_sq})")
        success = engine.make_move(from_sq, to_sq)
        
        if not success:
            print(f"ERROR: Engine rejected a move it generated itself! {move}")
            
    print("\nFinal Board State:")
    engine.print_board()
    print(f"Final FEN: {engine.get_fen()}")
    print("Demo finished successfully!")

if __name__ == "__main__":
    main()
