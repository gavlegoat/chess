#pragma once

#include <map>
#include <set>
#include <iostream>
#include <deque>

// There are 12 bitboards for the indivitual pieces plus 2 representing all of
// the pieces for each side and 1 representing all of the pieces for both
// sides.
#define NUM_BOARDS 15

/**
 * \brief A single move.
 *
 * A move consists of a starting and ending square along with the piece that
 * moved and some flags indicating what kind of move it was. Specifically, a
 * move may be:
 * - a capture (the captured piece needs to be removed),
 * - a double pawn push (en passant possibilities should be updated),
 * - a king or queenside castle (the associated rook needs to be moved),
 * - an en passant capture (the captured piece is not on the destination square),
 * - promotion (the pawn needs to be replaced with the specified piece),
 * - promotion with a capture, or
 * - quite (none of the above applies).
 */
class Move {
  private:
    int from_sq;      /**< The starting square of the move. */
    int to_sq;        /**< The ending square of the move. */
    int piece_moved;  /**< The piece which was moved. */
    uint16_t flags;   /**< Other information about the move. */

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

    /**
     * \brief Create a new move.
     *
     * \param from The starting square.
     * \param to The ending square.
     * \param p The piece which moved.
     * \param fl The move type flag.
     */
    Move(int from, int to, int p, uint16_t fl);

    /** \name Flag Tests
     * These functions test the move type flag.
     */
    ///@{

    /**
     * \brief True if this move is a kingside castle.
     */
    inline bool castle_kingside() const {
      return flags == KING_CASTLE;
    }

    /**
     * \brief True if this move is a queenside castle.
     */
    inline bool castle_queenside() const {
      return flags == QUEEN_CASTLE;
    }

    /**
     * \brief True if this move is a double pawn push.
     */
    inline bool double_pawn_push() const {
      return flags == PAWN_DOUBLE;
    }

    /**
     * \brief True if this move results in a capture.
     */
    inline bool capture() const {
      return (flags & 0x4) != 0;
    }

    /**
     * \brief True if this move is an en passant capture.
     */
    inline bool capture_ep() const {
      return flags == CAPTURE_EP;
    }

    /**
     * \brief True if this move ends in promotion to a knight.
     *
     * Note that this returns true both for normal promotion and promotion
     * with capture.
     */
    inline bool promote_knight() const {
      return flags == PROMOTE_KNIGHT || flags == PROMOTE_KNIGHT_CAPTURE;
    }

    /**
     * \brief True if this move ends in promotion to a bishop.
     *
     * Note that this returns true both for normal promotion and promotion
     * with capture.
     */
    inline bool promote_bishop() const {
      return flags == PROMOTE_BISHOP || flags == PROMOTE_BISHOP_CAPTURE;
    }

    /**
     * \brief True if this move ends in promotion to a rook.
     *
     * Note that this returns true both for normal promotion and promotion
     * with capture.
     */
    inline bool promote_rook() const {
      return flags == PROMOTE_ROOK || flags == PROMOTE_ROOK_CAPTURE;
    }

    /**
     * \brief True if this move ends in promotion to a queen.
     *
     * Note that this returns true both for normal promotion and promotion
     * with capture.
     */
    inline bool promote_queen() const {
      return flags == PROMOTE_QUEEN || flags == PROMOTE_QUEEN_CAPTURE;
    }

    ///@}

    /**
     * \brief Get the starting square of this move.
     */
    inline int from_square() const {
      return from_sq;
    }

    /**
     * \brief Get the ending square of this move.
     */
    inline int to_square() const {
      return to_sq;
    }

    /**
     * \brief Get the moved piece.
     */
    inline int piece() const {
      return piece_moved;
    }

    /**
     * \brief Get the flags for this move.
     *
     * It is generally better to use the flat test functions unless you are
     * using the flags to compare this move to another.
     */
    inline uint16_t get_flags() const {
      return flags;
    }

    /**
     * \brief Determine whether two moves are equal.
     */
    friend bool operator==(const Move& l, const Move& r) {
      return l.flags == r.flags && l.piece_moved == r.piece_moved &&
        l.from_sq == r.from_sq && l.to_sq == r.to_sq;
    }
};

/**
 * \brief A board position.
 *
 * The layout of pieces on a board is represented by a set of bitboards in
 * rank-major order where the most-significant bit is h8 and the least
 * significant bit is a1. The squares are numbered accordingly so that a1 is 0
 * and h8 is 63.
 */
class Position {
  private:
    /** The bitboards representing the position. */
    uint64_t boards[NUM_BOARDS];
    /** A cache of the locations of each piece type for fast access. */
    std::set<int> piece_sets[NUM_BOARDS];

  public:
    /**
     * \name Piece Labels
     *
     * These constants are used to identify different piece types. They come
     * in colored and generic varieties. For the most part, only the colored
     * versions should be used to update the position -- the generic ones
     * should only be used with Position::color_piece.
     */
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

    /**
     * \brief Construct a representation of the starting position.
     */
    Position();

    /**
     * \brief Construct a new position from a FEN string.
     *
     * Note that this constructor expects a _partial_ FEN string containing
     * only the part of the string which describes the board position. That is,
     * everything up to the first space in the string should be passed.
     */
    Position(std::string fen);

    /**
     * \brief Determine whether a given piece is white.
     */
    inline static bool piece_is_white(int piece) {
      return piece <= 6;
    }

    /**
     * \brief Give a color to a generic piece.
     *
     * Given one of the generic pieces (`PAWN`, `KNIGHT`, etc.) and a color,
     * return the identifier for pieces of that type and color (`W_PAWN`,
     * `B_KNIGHT`, etc.).
     *
     * \param piece A generic piece to add a color too.
     * \param is_white True to generate a white piece, false for black.
     * \return A piece of the specified color and type.
     */
    inline static int color_piece(int piece, bool is_white) {
      if (is_white) {
        return piece;
      } else {
        return piece + 7;
      }
    }

    /**
     * \brief Put a piece on the board.
     *
     * Note that this function does not check whether there is already a piece
     * on the specified square.
     *
     * \param pos The square to place a piece on.
     * \param piece The piece to place on the square.
     */
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
    /**
     * \brief Remove a piece from the board.
     *
     * Note that this function does not check whether the piece is actually on
     * the specified square. If there is a piece at the given square but it is
     * not the specified piece, the position can become corrupted.
     *
     * \param pos The square to remove a piece from.
     * \param piece The piece to remove.
     */
    inline void remove_piece(int pos, int piece) {
      uint64_t mask = ~(1ull << pos);
      boards[piece] &= mask;
      boards[W_ALL] &= mask;
      boards[B_ALL] &= mask;
      boards[BOTH_ALL] &= mask;
      piece_sets[piece].erase(pos);
    }

    /**
     * \brief Update the position by making a move.
     */
    void make_move(const Move& m);

    /**
     * \brief Determine whether the given piece is at the given square.
     */
    inline bool piece_at(int square, int piece) const {
      return (boards[piece] & (1ull << square)) != 0;
    }

    /**
     * \brief Get the positions of all pieces of a given type.
     */
    inline const std::set<int>& find_piece(int piece) const {
      return piece_sets[piece];
    }

    /**
     * \brief Get the bitboard for a particular piece.
     */
    inline uint64_t get_board(int piece) const {
      return boards[piece];
    }

    /**
     * \brief Generate a FEN string for this board.
     */
    std::string fen_board() const;

    /**
     * \brief Impose an arbitrary ordering on positions.
     *
     * This ordering is only provided to allow positions to be used as the key
     * of a map.
     */
    bool operator<(const Position& other) const;
};

/**
 * \brief A part of the game state.
 *
 * A node holds the space-efficient parts of the game state. The point here is
 * to allow fast making and undoing of moves by copying the small parts of the
 * game state to reduce the computation required to undo a move.
 */
class Node {
  public:
    Position position;    /**< The current board state. */
    bool white_to_move;   /**< true if it's white's move, false if it's black's. */
    bool w_castle_k;      /**< White can castle kingside. */
    bool w_castle_q;      /**< White can castle queenside. */
    bool b_castle_k;      /**< Black can castle kingside. */
    bool b_castle_q;      /**< Black can castle queenside. */
    uint64_t en_passant_square;   /**< The square where en passant can occur. */
    bool en_passant_possible;     /**< True if an en passant move is legal. */
    int half_moves_since_reset;   /**< Number of moves since pawn move or capture. */
    int moves;    /**< Current move number (1 in the initial position). */

    /**
     * \brief Construct a node representing the starting position.
     */
    Node();

    /**
     * \brief Construct a node with the given features.
     *
     * \param pos The board position.
     * \param wtm True if its white's turn to move.
     * \param wck True if white can castle kingside.
     * \param wcq True if white can castle queenside.
     * \param bck True if black can castle kingside.
     * \param bcq True if black can castle queenside.
     * \param eps The square where en passant can occure.
     * \param epp True if en passant is legal.
     * \param msr Half-moves since the last pawn move or piece capture.
     * \param ms The current move number.
     */
    Node(Position pos, bool wtm, bool wck, bool wcq, bool bck, bool bcq,
        uint64_t eps, bool epp, int msr, int ms);
};

/**
 * \brief The state of a chess game.
 */
class GameState {
  private:
    /// Most features of the current position.
    Node node;
    /** A count of the number of time each position has been reached. This is
     * used for determining when the game is drawn by repetition. */
    std::map<Position, int> repeats;
    /// A history of nodes, used for quickly undoing moves.
    std::deque<Node> history;

  public:
    /**
     * \brief Construct the initial game state.
     */
    GameState();

    /**
     * \brief Construct a game state from a FEN string.
     */
    GameState(std::string fen);

    /**
     * \brief Construct a game state from its constituent pieces.
     */
    GameState(Position pos, bool wtm, bool wck, bool wcq, bool bck, bool bcq,
        uint64_t eps, bool epp, int msr, int ms, std::map<Position, int> rs,
        std::deque<Node> history);

    /**
     * \brief Determine whether it is white's turn to move.
     */
    inline bool whites_move() const {
      return node.white_to_move;
    }

    /**
     * \brief Get the current position of the board.
     */
    inline const Position& pos() const {
      return node.position;
    }

    /**
     * \brief Determine whether en passant is possible.
     */
    inline bool en_passant() const {
      return node.en_passant_possible;
    }

    /**
     * \brief Return the square to which the capturing pawn can go for en passant.
     */
    inline int en_passant_target() const {
      return node.en_passant_square;
    }

    /**
     * \brief Return a bitboard of square the king moves through to castle kingside.
     *
     * This includes the square the king starts on and it useful for checking
     * whether there are any checks preventing castling.
     */
    inline uint64_t castle_through_kingside() const {
      if (node.white_to_move && node.w_castle_k) {
        return (1ull << 4) | (1ull << 5) | (1ull << 6);
      } else if (!node.white_to_move && node.b_castle_k) {
        return (1ull << 60) | (1ull << 61) | (1ull << 62);
      }
      return 0;
    }

    /**
     * \brief Return a bitboard of square the king moves through to castle queenside.
     *
     * This includes the square the king starts on and it useful for checking
     * whether there are any checks preventing castling.
     */
    inline uint64_t castle_through_queenside() const {
      if (node.white_to_move && node.w_castle_q) {
        return (1ull << 2) | (1ull << 3) | (1ull << 4);
      } else if (!node.white_to_move && node.b_castle_q) {
        return (1ull << 58) | (1ull << 59) | (1ull << 60);
      }
      return 0;
    }

    /**
     * \brief Make a move.
     */
    void make_move(const Move& m);

    /**
     * \brief Make a null move.
     *
     * This changes who's turn it is to move without actually changing the
     * position. This is useful for a variety of internal implementations as
     * well as sometimes speeding up alpha-beta search.
     */
    void flip_move();

    /**
     * \brief Undo the last move.
     */
    void undo_move();

    /**
     * \brief Generate a FEN string for this game state.
     */
    std::string fen_string() const;

    /**
     * \brief Write this game state's FEN string to an output stream.
     */
    friend std::ostream& operator<<(std::ostream& os, const GameState& gs);
};
