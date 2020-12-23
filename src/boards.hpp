#pragma once

#include <map>
#include <set>
#include <iostream>
#include <deque>

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

    inline uint16_t get_flags() const {
      return flags;
    }

    friend bool operator==(const Move& l, const Move& r) {
      return l.flags == r.flags && l.piece_moved == r.piece_moved &&
        l.from_sq == r.from_sq && l.to_sq == r.to_sq;
    }
};

// A Position represents a layout of pieces on a board. Each bitboard is stored
// with the MSB representing h8 and the LSB representing a1. When we represent
// a square, we do it with an integer s such that (0x1 << s) masks the
// appropriate bit. Then a1 is represented by 0 and h8 by 63.
class Position {
  private:
    uint64_t boards[NUM_BOARDS];
    std::set<int> piece_sets[NUM_BOARDS];

  public:
    static const int W_PAWN   = 0;
    static const int W_KNIGHT = 1;
    static const int W_BISHOP = 2;
    static const int W_ROOK   = 3;
    static const int W_QUEEN  = 4;
    static const int W_KING   = 5;
    static const int W_ALL    = 6;
    static const int B_PAWN   = 7;
    static const int B_KNIGHT = 8;
    static const int B_BISHOP = 9;
    static const int B_ROOK   = 10;
    static const int B_QUEEN  = 11;
    static const int B_KING   = 12;
    static const int B_ALL    = 13;
    static const int BOTH_ALL = 14;

    static const int PAWN = 0;
    static const int KNIGHT = 1;
    static const int BISHOP = 2;
    static const int ROOK = 3;
    static const int QUEEN = 4;
    static const int KING = 5;
    static const int ALL = 6;

    Position();
    Position(std::string fen);

    inline static bool piece_is_white(int piece) {
      return piece <= 6;
    }

    inline static int color_piece(int piece, bool is_white) {
      if (is_white) {
        return piece;
      } else {
        return piece + 7;
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

class Node {
  public:
    Position position;    // The current board state
    bool white_to_move;   // true if it's whites move, false if it's black's
    bool w_castle_k;      // White can castle kingside
    bool w_castle_q;      // White can castle queenside
    bool b_castle_k;      // Black can castle kingside
    bool b_castle_q;      // Black can castle queenside
    uint64_t en_passant_square;   // The square where en passant can occur
    bool en_passant_possible;      // true if an en passant move is legal
    int half_moves_since_reset;        // Number of moves since pawn move or capture
    int moves;      // Current move number (1 in the initial position)

    Node();

    Node(Position pos, bool wtm, bool wck, bool wcq, bool bck, bool bcq,
        uint64_t eps, bool epp, int msr, int ms);
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
    Node node;
    std::map<Position, int> repeats;   // Repeated positions
    std::deque<Node> history;

  public:
    // Create the initial game state
    GameState();

    // Create a game state from a FEN string
    GameState(std::string fen);

    // Fully general constructor
    GameState(Position pos, bool wtm, bool wck, bool wcq, bool bck, bool bcq,
        uint64_t eps, bool epp, int msr, int ms, std::map<Position, int> rs,
        std::deque<Node> history);

    inline bool whites_move() const {
      return node.white_to_move;
    }

    inline const Position& pos() const {
      return node.position;
    }

    inline bool en_passant() const {
      return node.en_passant_possible;
    }

    inline int en_passant_target() const {
      return node.en_passant_square;
    }

    // Get a bitboard of squares we need to look at for checks when castling.
    // Returns 0 if castling flags are false.
    inline uint64_t castle_through_kingside() const {
      if (node.white_to_move && node.w_castle_k) {
        return (1ull << 4) | (1ull << 5) | (1ull << 6);
      } else if (!node.white_to_move && node.b_castle_k) {
        return (1ull << 60) | (1ull << 61) | (1ull << 62);
      }
      return 0;
    }

    inline uint64_t castle_through_queenside() const {
      if (node.white_to_move && node.w_castle_q) {
        return (1ull << 2) | (1ull << 3) | (1ull << 4);
      } else if (!node.white_to_move && node.b_castle_q) {
        return (1ull << 58) | (1ull << 59) | (1ull << 60);
      }
      return 0;
    }

    // Make a move
    void make_move(const Move& m);

    void undo_move(const Move& m);

    std::string fen_string() const;

    friend std::ostream& operator<<(std::ostream& os, const GameState& gs);
};
