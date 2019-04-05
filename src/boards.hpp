#ifndef _BOARDS_H_
#define _BOARDS_H_

#include <map>
#include <set>
#include <iostream>

#define NUM_BOARDS 15

// A Move represents some move. This is characterized by the beginning and end
// squares of the move along with a number of flags describing other conditions
// that may apply (e.g., whether this move was a capture).
class Move {
  private:
    int from_sq;
    int to_sq;
    int piece_moved;
    uint16_t flags;

  public:
    static const uint16_t QUIET = 0;
    static const uint16_t PAWN_DOUBLE = 1;
    static const uint16_t KING_CASTLE = 2;
    static const uint16_t QUEEN_CASTLE = 3;
    static const uint16_t CAPTURE = 4;
    static const uint16_t CAPTURE_EP = 5;
    static const uint16_t PROMOTE_KNIGHT = 8;
    static const uint16_t PROMOTE_BISHOP = 9;
    static const uint16_t PROMOTE_ROOK = 10;
    static const uint16_t PROMOTE_QUEEN = 11;
    static const uint16_t PROMOTE_KNIGHT_CAPTURE = 12;
    static const uint16_t PROMOTE_BISHOP_CAPTURE = 13;
    static const uint16_t PROMOTE_ROOK_CAPTURE = 14;
    static const uint16_t PROMOTE_QUEEN_CAPTURE = 15;

    Move(int from, int to, int p, uint16_t fl);

    inline bool castle_kingside() const {
      return flags == KING_CASTLE;
    }

    inline bool castle_queenside() const {
      return flags == QUEEN_CASTLE;
    }

    inline bool double_pawn_push() const {
      return flags == PAWN_DOUBLE;
    }

    inline bool capture() const {
      return (flags & 0x4) != 0;
    }

    inline bool capture_ep() const {
      return flags == CAPTURE_EP;
    }

    inline bool promote_knight() const {
      return flags == PROMOTE_KNIGHT || flags == PROMOTE_KNIGHT_CAPTURE;
    }

    inline bool promote_bishop() const {
      return flags == PROMOTE_BISHOP || flags == PROMOTE_BISHOP_CAPTURE;
    }

    inline bool promote_rook() const {
      return flags == PROMOTE_ROOK || flags == PROMOTE_ROOK_CAPTURE;
    }

    inline bool promote_queen() const {
      return flags == PROMOTE_QUEEN || flags == PROMOTE_QUEEN_CAPTURE;
    }

    inline int from_square() const {
      return from_sq;
    }

    inline int to_square() const {
      return to_sq;
    }

    inline int piece() const {
      return piece_moved;
    }
};

// A Position represents a layout of pieces on a board. Each bitboard is stored
// with the MSB representing h8 and the LSB representing a1. When we represent
// a square, we do it with an integer s such that (0x1 << s) masks the
// appropriate bit. Then a1 is represented by 0 and h8 by 63.
class Position {
  private:
    uint64_t boards[NUM_BOARDS];
    std::set<int> piece_sets[12];

  public:
    static const int W_PAWN   = 0;
    static const int W_KNIGHT = 1;
    static const int W_BISHOP = 2;
    static const int W_ROOK   = 3;
    static const int W_QUEEN  = 4;
    static const int W_KING   = 5;
    static const int B_PAWN   = 6;
    static const int B_KNIGHT = 7;
    static const int B_BISHOP = 8;
    static const int B_ROOK   = 9;
    static const int B_QUEEN  = 10;
    static const int B_KING   = 11;
    static const int W_ALL    = 12;
    static const int B_ALL    = 13;
    static const int BOTH_ALL = 14;

    static const int PAWN = 0;
    static const int KNIGHT = 1;
    static const int BISHOP = 2;
    static const int ROOK = 3;
    static const int QUEEN = 4;
    static const int KING = 5;

    Position();
    Position(std::string fen);

    inline static bool piece_is_white(int piece) {
      return piece <= 5;
    }

    inline static int color_piece(int piece, bool is_white) {
      if (is_white) {
        return piece;
      } else {
        return piece + 6;
      }
    }

    // Put a piece on the board. For the moment this is trivial but it might
    // be useful to have this as a separate method if the board representation
    // gets more complicated.
    inline void place_piece(int pos, int piece) {
      uint64_t mask = 1ull << pos;
      boards[piece] |= mask;
      boards[BOTH_ALL] |= mask;
      if (piece_is_white(piece)) {
        boards[W_ALL] |= mask;
      } else {
        boards[B_ALL] |= mask;
      }
      piece_sets[piece].insert(pos);
    }

    // Remove a piece from the board
    inline void remove_piece(int pos, int piece) {
      uint64_t mask = ~(1ull << pos);
      boards[piece] &= mask;
      boards[W_ALL] &= mask;
      boards[B_ALL] &= mask;
      boards[BOTH_ALL] &= mask;
      piece_sets[piece].erase(pos);
    }

    // Make a move, updating the position
    void make_move(const Move& m);

    // Determine whether the given piece is at the given square
    inline bool piece_at(int square, int piece) const {
      return (boards[piece] & (1ull << square)) != 0;
    }

    inline const std::set<int>& find_piece(int piece) const {
      return piece_sets[piece];
    }

    inline uint64_t get_board(int piece) const {
      return boards[piece];
    }

    std::string fen_board() const;

    // We just need an arbitrary ordering for data structures
    bool operator<(const Position& other) const;
};

// A GameState represents:
// - The current board position
// - The player to move
// - The castling posibilities
// - The en passant square, if any
// - A count of moves since the last pawn move or capture
// - A count of the number of times each position has been seen, for
//   looking for draws by repetition.
class GameState {
  private:
    Position position;    // The current board state
    bool white_to_move;   // true if it's whites move, false if it's black's
    bool w_castle_k;      // White can castle kingside
    bool w_castle_q;      // White can castle queenside
    bool b_castle_k;      // Black can castle kingside
    bool b_castle_q;      // Black can castle queenside
    int en_passant_square;   // The square where en passant can occur
    bool en_passant_possible;      // true if an en passant move is legal
    int half_moves_since_reset;        // Number of moves since pawn move or capture
    int moves;      // Current move number (1 in the initial position)
    std::map<Position, int> repeats;   // Repeated positions

  public:
    // Create the initial game state
    GameState();

    // Create a game state from a FEN string
    GameState(std::string fen);

    // Fully general constructor
    GameState(Position pos, bool wtm, bool wck, bool wcq, bool bck, bool bcq,
        uint64_t eps, bool epp, int msr, int ms, std::map<Position, int> rs);

    inline bool whites_move() const {
      return white_to_move;
    }

    inline const Position& pos() const {
      return position;
    }

    // Make a move
    void make_move(const Move& m);

    std::string fen_string() const;

    friend std::ostream& operator<<(std::ostream& os, const GameState& gs);
};

#endif
