#include "movegen.hpp"
#include "utils.hpp"

#include <cstdlib>
#include <set>
#include <random>

// A brief explanation of magic bitboards: When we need to figure out where
// sliding pieces move, we need to consider many possible configurations of
// occupancy (that is, pieces blocking the way). A magic bitboard allows us to
// do this quickly and (relatively) space efficiently. The idea is to intersect
// the squares we care about with the occupancy map of the board to get a
// (64-bit) occupancy mask. While this is a 64-bit value, typically fewer than
// 12 of the bits are relevant. We then look for a magic number such that
// multiplying the occupancy mask by the magic number always gives us a
// consecutive sequence of relevant bits surrounded by irrelevant bits. Then we
// can shift this resulting value so that the relevant bits are the lowest bits
// and use it to index a table of legal moves. This way we can use at most 13
// index bits to store all of the relevant moves, rather than 64.
// Given a Magic object m and a piece occupancy bitboard occ for the whole
// board, we compute the possible attacks as
// attack_table[((occ & m.mask) * magic) >> (64 - shift)]

/**
 * \brief A magic bitboard.
 */
typedef struct {
  /// The generated magic number.
  uint64_t magic;
  /// The occlusion mask.
  uint64_t mask;
  /// An array of attack boards.
  uint64_t* attack_table;
  /// The shift to apply.
  int shift;
} Magic;

/// A set of masks showing where the knight can move to from each square.
uint64_t knight_moves[64];
/// A set of masks showing where the king can move to from each square.
uint64_t king_moves[64];

/// A set of magic bitboards describing rook moves form each square.
Magic rook_magics[64];
/// A set of magic bitboards describing bishop moves form each square.
Magic bishop_magics[64];

// Get a board representing all of the pieces which can move to the given
// target. Note that this move only considers moves which end with a piece
// on the target square. In particular, if there is a pawn on the target
// square which can be taken en passant, the capturing pawn will not be
// indicated on the returned board.
uint64_t get_attacks_to(const Position& p, int target, bool white_to_move,
    uint64_t occupancy) {
  int opp_knight = Position::color_piece(Position::KNIGHT, !white_to_move);
  int opp_bishop = Position::color_piece(Position::BISHOP, !white_to_move);
  int opp_rook = Position::color_piece(Position::ROOK, !white_to_move);
  int opp_queen = Position::color_piece(Position::QUEEN, !white_to_move);
  int opp_king = Position::color_piece(Position::KING, !white_to_move);
  uint64_t knight_board = p.get_board(opp_knight);
  uint64_t bishop_board = p.get_board(opp_bishop);
  uint64_t rook_board = p.get_board(opp_rook);
  uint64_t queen_board = p.get_board(opp_queen);
  uint64_t king_board = p.get_board(opp_king);
  Magic bm = bishop_magics[target];
  uint64_t bishop_attack = bm.attack_table[((occupancy & bm.mask) * bm.magic)
    >> (64 - bm.shift)];
  Magic rm = rook_magics[target];
  uint64_t rook_attack = rm.attack_table[((occupancy & rm.mask) * rm.magic)
    >> (64 - rm.shift)];
  uint64_t check_board = knight_moves[target] & knight_board;
  check_board |= king_moves[target] & king_board;
  check_board |= bishop_attack & (bishop_board | queen_board);
  check_board |= rook_attack & (rook_board | queen_board);

  int opp_pawn = Position::color_piece(Position::PAWN, !white_to_move);
  int krank = target / 8;
  int kfile = target % 8;
  if (kfile < 7) {
    if (white_to_move && krank < 7) {
      if (p.piece_at(target + 9, opp_pawn)) {
        check_board |= 1ull << (target + 9);
      }
    } else if (krank > 0) {
      if (p.piece_at(target - 7, opp_pawn)) {
        check_board |= 1ull << (target - 7);
      }
    }
  }
  if (kfile > 0) {
    if (white_to_move && krank < 7) {
      if (p.piece_at(target + 7, opp_pawn)) {
        check_board |= 1ull << (target + 7);
      }
    } else if (krank > 0) {
      if (p.piece_at(target - 9, opp_pawn)) {
        check_board |= 1ull << (target - 9);
      }
    }
  }
  return check_board;
}

// Generate a bitboard of all checking pieces
uint64_t get_check_board(bool white_to_move, const Position& p) {
  int king = Position::color_piece(Position::KING, white_to_move);
  int king_sq = *p.find_piece(king).begin();
  uint64_t occupancy = p.get_board(Position::BOTH_ALL);
  return get_attacks_to(p, king_sq, white_to_move, occupancy);
}

bool in_check(bool white_to_move, const Position& p) {
  return !(get_check_board(white_to_move, p) == 0);
}

// Count the number of 1 bits in a number
int popcount(uint64_t x) {
  int count = 0;
  while (x != 0) {
    count++;
    x = x & (x - 1);
  }
  return count;
}

#ifdef __GNUC__
inline int lsb(uint64_t x) {
  return __builtin_ctzll(x);
}
#else
inline int lsb(uint64_t x) {
  uint64_t y = x - 1;
  x = (x | y) ^ y;
  return std::log2(x);
}
#endif

// Append all of the moves for a given piece type starting from some square.
void append_moves_from(int from_square, uint64_t to_squares,
    int piece, uint64_t opp_pieces, MoveList& l) {
  while (to_squares != 0) {
    int tsq = lsb(to_squares);
    to_squares &= to_squares - 1;
    if (opp_pieces & (1ull << tsq)) {
      l.push_back(Move(from_square, tsq, piece, Move::CAPTURE));
    } else {
      l.push_back(Move(from_square, tsq, piece, Move::QUIET));
    }
  }
}

// Generate pseudolegal king moves
MoveList generate_king_moves(const GameState& gs) {
  const Position& p = gs.pos();
  bool white_to_move = gs.whites_move();
  int our_king = Position::color_piece(Position::KING, white_to_move);
  int our_pieces = Position::color_piece(Position::ALL, white_to_move);
  int king_square = *p.find_piece(our_king).begin();
  uint64_t king_move_board = king_moves[king_square];
  king_move_board &= ~p.get_board(our_pieces);
  int opp_pieces = Position::color_piece(Position::ALL, !white_to_move);
  uint64_t opp_all_board = p.get_board(opp_pieces);
  MoveList ret;
  append_moves_from(king_square, king_move_board, our_king, opp_all_board, ret);
  return ret;
}

void append_castling_moves(const GameState& gs, MoveList& l) {
  const Position& p = gs.pos();
  bool white_to_move = gs.whites_move();
  int our_king = Position::color_piece(Position::KING, white_to_move);
  uint64_t king_board = p.get_board(our_king);
  uint64_t squares_to_check = gs.castle_through_kingside();
  uint64_t occupancy = p.get_board(Position::BOTH_ALL);
  if (squares_to_check != 0) {
    bool can_castle = true;
    while (squares_to_check != 0) {
      int sq = lsb(squares_to_check);
      squares_to_check &= squares_to_check - 1;
      uint64_t attacks = get_attacks_to(p, sq, white_to_move, occupancy);
      if (attacks != 0 || ((occupancy & (1ull << sq)) && !(king_board & (1ull << sq)))) {
        can_castle = false;
        break;
      }
    }
    if (can_castle) {
      if (white_to_move && p.piece_at(4, Position::W_KING) &&
          p.piece_at(7, Position::W_ROOK)) {
        l.push_back(Move(4, 6, Position::W_KING, Move::KING_CASTLE));
      } else if (!white_to_move && p.piece_at(60, Position::B_KING) &&
          p.piece_at(63, Position::B_ROOK)) {
        l.push_back(Move(60, 62, Position::B_KING, Move::KING_CASTLE));
      }
    }
  }
  squares_to_check = gs.castle_through_queenside();
  if (squares_to_check != 0) {
    bool can_castle = true;
    while (squares_to_check != 0) {
      int sq = lsb(squares_to_check);
      squares_to_check &= squares_to_check - 1;
      uint64_t attacks = get_attacks_to(p, sq, white_to_move, occupancy);
      if (attacks != 0 || ((occupancy & (1ull << sq)) && !(king_board & (1ull << sq)))) {
        can_castle = false;
        break;
      }
    }
    if (can_castle) {
      if (white_to_move && p.piece_at(4, Position::W_KING) &&
          p.piece_at(0, Position::W_ROOK) && (occupancy & (1ull << 1)) == 0) {
        l.push_back(Move(4, 2, Position::W_KING, Move::QUEEN_CASTLE));
      } else if (!white_to_move && p.piece_at(60, Position::B_KING) &&
          p.piece_at(56, Position::B_ROOK) && (occupancy & (1ull << 57)) == 0) {
        l.push_back(Move(60, 58, Position::B_KING, Move::QUEEN_CASTLE));
      }
    }
  }
}

void append_en_passant(const GameState& gs, MoveList& l) {
  if (!gs.en_passant()) {
    return;
  }
  int square = gs.en_passant_target();
  bool white_to_move = gs.whites_move();
  const Position& p = gs.pos();
  int our_pawn = Position::color_piece(Position::PAWN, white_to_move);
  if (white_to_move) {
    if (p.piece_at(square - 9, our_pawn) && square % 8 > 0) {
      l.push_back(Move(square - 9, square, our_pawn, Move::CAPTURE_EP));
    }
    if (p.piece_at(square - 7, our_pawn) && square % 8 < 7) {
      l.push_back(Move(square - 7, square, our_pawn, Move::CAPTURE_EP));
    }
  } else {
    if (p.piece_at(square + 9, our_pawn) && square % 8 < 7) {
      l.push_back(Move(square + 9, square, our_pawn, Move::CAPTURE_EP));
    }
    if (p.piece_at(square + 7, our_pawn) && square % 8 > 0) {
      l.push_back(Move(square + 7, square, our_pawn, Move::CAPTURE_EP));
    }
  }
}

void append_pawn_moves(const GameState& gs, MoveList& l) {
  bool white_to_move = gs.whites_move();
  int our_pawn = Position::color_piece(Position::PAWN, white_to_move);
  const Position& p = gs.pos();
  const std::set<int>& pawns = p.find_piece(our_pawn);
  int opp_all = Position::color_piece(Position::ALL, !white_to_move);
  uint64_t opp_pieces = p.get_board(opp_all);
  uint64_t all_pieces = p.get_board(Position::BOTH_ALL);
  for (int p : pawns) {
    int target = white_to_move ? p + 8 : p - 8;
    if ((all_pieces & (1ull << target)) == 0) {
      if (target / 8 == 0 || target / 8 == 7) {
        l.push_back(Move(p, target, our_pawn, Move::PROMOTE_KNIGHT));
        l.push_back(Move(p, target, our_pawn, Move::PROMOTE_BISHOP));
        l.push_back(Move(p, target, our_pawn, Move::PROMOTE_ROOK));
        l.push_back(Move(p, target, our_pawn, Move::PROMOTE_QUEEN));
      } else {
        l.push_back(Move(p, target, our_pawn, Move::QUIET));
      }
    }
    target--;   // pawn + 7 for white, pawn - 9 for black
    if (p % 8 > 0 && ((opp_pieces & (1ull << target)) != 0)) {
      if (target / 8 == 0 || target / 8 == 7) {
        l.push_back(Move(p, target, our_pawn, Move::PROMOTE_KNIGHT_CAPTURE));
        l.push_back(Move(p, target, our_pawn, Move::PROMOTE_BISHOP_CAPTURE));
        l.push_back(Move(p, target, our_pawn, Move::PROMOTE_ROOK_CAPTURE));
        l.push_back(Move(p, target, our_pawn, Move::PROMOTE_QUEEN_CAPTURE));
      } else {
        l.push_back(Move(p, target, our_pawn, Move::CAPTURE));
      }
    }
    target += 2;
    if (p % 8 < 7 && ((opp_pieces & (1ull << target)) != 0)) {
      if (target / 8 == 0 || target / 8 == 7) {
        l.push_back(Move(p, target, our_pawn, Move::PROMOTE_KNIGHT_CAPTURE));
        l.push_back(Move(p, target, our_pawn, Move::PROMOTE_BISHOP_CAPTURE));
        l.push_back(Move(p, target, our_pawn, Move::PROMOTE_ROOK_CAPTURE));
        l.push_back(Move(p, target, our_pawn, Move::PROMOTE_QUEEN_CAPTURE));
      } else {
        l.push_back(Move(p, target, our_pawn, Move::CAPTURE));
      }
    }
    if (white_to_move ? p / 8 == 1 : p / 8 == 6) {
      target = white_to_move ? p + 16 : p - 16;
      int sq = white_to_move ? p + 8 : p - 8;
      if ((all_pieces & ((1ull << target) | (1ull << sq))) == 0) {
        l.push_back(Move(p, target, our_pawn, Move::PAWN_DOUBLE));
      }
    }
  }
}

void append_knight_moves(const GameState& gs, MoveList& l) {
  bool white_to_move = gs.whites_move();
  int our_knight = Position::color_piece(Position::KNIGHT, white_to_move);
  const Position& p = gs.pos();
  const std::set<int>& knights = p.find_piece(our_knight);
  int our_all = Position::color_piece(Position::ALL, white_to_move);
  int opp_all = Position::color_piece(Position::ALL, !white_to_move);
  uint64_t occupancy = p.get_board(our_all);
  uint64_t opp_pieces = p.get_board(opp_all);
  for (int k : knights) {
    uint64_t moves_to = knight_moves[k] & ~occupancy;
    append_moves_from(k, moves_to, our_knight, opp_pieces, l);
  }
}

void append_sliding_moves(const GameState& gs, MoveList& l) {
  bool white_to_move = gs.whites_move();
  const Position& p = gs.pos();
  uint64_t occupancy = p.get_board(Position::BOTH_ALL);
  int our_all = Position::color_piece(Position::ALL, white_to_move);
  uint64_t our_pieces = p.get_board(our_all);
  int opp_all = Position::color_piece(Position::ALL, !white_to_move);
  uint64_t opp_pieces = p.get_board(opp_all);
  int our_rook = Position::color_piece(Position::ROOK, white_to_move);
  const std::set<int>& rooks = p.find_piece(our_rook);
  for (int r : rooks) {
    Magic rm = rook_magics[r];
    uint64_t r_att = rm.attack_table[((occupancy & rm.mask) * rm.magic) >> (64 - rm.shift)];
    uint64_t targets = r_att & ~our_pieces;
    append_moves_from(r, targets, our_rook, opp_pieces, l);
  }
  int our_bishop = Position::color_piece(Position::BISHOP, white_to_move);
  const std::set<int>& bishops = p.find_piece(our_bishop);
  for (int b : bishops) {
    Magic bm = bishop_magics[b];
    uint64_t b_att = bm.attack_table[((occupancy & bm.mask) * bm.magic) >> (64 - bm.shift)];
    uint64_t targets = b_att & ~our_pieces;
    append_moves_from(b, targets, our_bishop, opp_pieces, l);
  }
  int our_queen = Position::color_piece(Position::QUEEN, white_to_move);
  const std::set<int>& queens = p.find_piece(our_queen);
  for (int q : queens) {
    Magic rm = rook_magics[q];
    Magic bm = bishop_magics[q];
    uint64_t r_att = rm.attack_table[((occupancy & rm.mask) * rm.magic) >> (64 - rm.shift)];
    uint64_t b_att = bm.attack_table[((occupancy & bm.mask) * bm.magic) >> (64 - bm.shift)];
    uint64_t targets = (r_att | b_att) & ~our_pieces;
    append_moves_from(q, targets, our_queen, opp_pieces, l);
  }
}

// Generate the pseudo-legal moves in this position. We will need to filter out
// moves that result in check later.
MoveList generate_pseudolegal_moves(const GameState& gs) {
  MoveList ret = generate_king_moves(gs);
  append_castling_moves(gs, ret);
  append_en_passant(gs, ret);
  append_pawn_moves(gs, ret);
  append_knight_moves(gs, ret);
  append_sliding_moves(gs, ret);

  return ret;
}

// Check whether a given pseudolegal move results in check
bool is_legal(const Move& m, const GameState& gs) {
  Position p(gs.pos());
  p.make_move(m);
  return get_check_board(gs.whites_move(), p) == 0;
}

MoveList generate_moves(const GameState& gs) {
  MoveList pseudo = generate_pseudolegal_moves(gs);
  MoveList::iterator it = pseudo.begin();
  while (it != pseudo.end()) {
    if (is_legal(*it, gs)) {
      it++;
    } else {
      it = pseudo.erase(it);
    }
  }
  return pseudo;
}

uint64_t generate_occupancy_mask(int s, bool is_rook) {
  uint64_t occupancy = 0;
  int rank = s / 8;
  int file = s % 8;
  int nrank, nfile, square;
  if (is_rook) {
    nrank = rank + 1;
    nfile = file;
    while (nrank < 7) {
      square = 8 * nrank + nfile;
      occupancy |= 1ull << square;
      nrank++;
    }
    nrank = rank - 1;
    while (nrank > 0) {
      square = 8 * nrank + nfile;
      occupancy |= 1ull << square;
      nrank--;
    }
    nrank = rank;
    nfile = file + 1;
    while (nfile < 7) {
      square = 8 * nrank + nfile;
      occupancy |= 1ull << square;
      nfile++;
    }
    nfile = file - 1;
    while (nfile > 0) {
      square = 8 * nrank + nfile;
      occupancy |= 1ull << square;
      nfile--;
    }
  } else {
    nrank = rank + 1;
    nfile = file + 1;
    while (nfile < 7 && nrank < 7) {
      square = 8 * nrank + nfile;
      occupancy |= 1ull << square;
      nrank++;
      nfile++;
    }
    nrank = rank + 1;
    nfile = file - 1;
    while (nfile > 0 && nrank < 7) {
      square = 8 * nrank + nfile;
      occupancy |= 1ull << square;
      nrank++;
      nfile--;
    }
    nrank = rank - 1;
    nfile = file + 1;
    while (nfile < 7 && nrank > 0) {
      square = 8 * nrank + nfile;
      occupancy |= 1ull << square;
      nrank--;
      nfile++;
    }
    nrank = rank - 1;
    nfile = file - 1;
    while (nfile > 0 && nrank > 0) {
      square = 8 * nrank + nfile;
      occupancy |= 1ull << square;
      nrank--;
      nfile--;
    }
  }
  return occupancy;
}

std::vector<uint64_t> generate_possible_occupancies(int sq,
    uint64_t occupancy, uint64_t partial, int ind) {
  if (ind >= 64) {
    return std::vector<uint64_t>{partial};
  }

  std::vector<uint64_t> zero = generate_possible_occupancies(sq, occupancy,
      partial, ind + 1);

  if ((occupancy & (1ull << ind))) {
    std::vector<uint64_t> one = generate_possible_occupancies(sq, occupancy,
        partial | (1ull << ind), ind + 1);
    zero.insert(zero.end(), one.begin(), one.end());
  }
  return zero;
}

// Generate the possible moves of a sliding piece given an occupancy mask. Note
// that the generated attack will include the first occupied square in each
// direction.
uint64_t generate_attack(int sq, uint64_t occupancy, bool is_rook) {
  int rank = sq / 8;
  int file = sq % 8;
  int nrank, nfile, square;
  uint64_t attack = 0;
  if (is_rook) {
    nrank = rank + 1;
    nfile = file;
    while (nrank < 8) {
      square = 8 * nrank + nfile;
      attack |= 1ull << square;
      if ((1ull << square) & occupancy) {
        break;
      }
      nrank++;
    }
    nrank = rank - 1;
    while (nrank >= 0) {
      square = 8 * nrank + nfile;
      attack |= 1ull << square;
      if ((1ull << square) & occupancy) {
        break;
      }
      nrank--;
    }
    nrank = rank;
    nfile = file + 1;
    while (nfile < 8) {
      square = 8 * nrank + nfile;
      attack |= 1ull << square;
      if ((1ull << square) & occupancy) {
        break;
      }
      nfile++;
    }
    nfile = file - 1;
    while (nfile >= 0) {
      square = 8 * nrank + nfile;
      attack |= 1ull << square;
      if ((1ull << square) & occupancy) {
        break;
      }
      nfile--;
    }
  } else {
    nrank = rank + 1;
    nfile = file + 1;
    while (nfile < 8 && nrank < 8) {
      square = 8 * nrank + nfile;
      attack |= 1ull << square;
      if ((1ull << square) & occupancy) {
        break;
      }
      nrank++;
      nfile++;
    }
    nrank = rank + 1;
    nfile = file - 1;
    while (nfile >= 0 && nrank < 8) {
      square = 8 * nrank + nfile;
      attack |= 1ull << square;
      if ((1ull << square) & occupancy) {
        break;
      }
      nrank++;
      nfile--;
    }
    nrank = rank - 1;
    nfile = file + 1;
    while (nfile < 8 && nrank >= 0) {
      square = 8 * nrank + nfile;
      attack |= 1ull << square;
      if ((1ull << square) & occupancy) {
        break;
      }
      nrank--;
      nfile++;
    }
    nrank = rank - 1;
    nfile = file - 1;
    while (nfile >= 0 && nrank >= 0) {
      square = 8 * nrank + nfile;
      attack |= 1ull << square;
      if ((1ull << square) & occupancy) {
        break;
      }
      nrank--;
      nfile--;
    }
  }
  return attack;
}

// Generate a magic number for some square
Magic generate_magic(int square, int shift, bool is_rook) {
  uint64_t occupancy = generate_occupancy_mask(square, is_rook);
  std::vector<uint64_t> occupancies = generate_possible_occupancies(square,
      occupancy, 0, 0);
  std::vector<uint64_t> attacks;
  for (unsigned i = 0; i < occupancies.size(); i++) {
    attacks.push_back(generate_attack(square, occupancies[i], is_rook));
  }

  // table will be the attack table of the generated Magic object
  uint64_t* table = (uint64_t*) malloc((1 << shift) * sizeof(uint64_t));

  std::random_device rd;
  std::mt19937_64 gen(rd());
  std::uniform_int_distribution<uint64_t> dis;
  while (true) {
    for (int i = 0; i < 1 << shift; i++) {
      // We initialize each attack in the table to all ones to check for collisions
      table[i] = ~0ull;
    }

    // Generate a random candidate magic number
    // In general we want to have a small number of 1 bits, so we'll & together
    // a few random numbers to bias the generator toward good numbers.
    uint64_t m = dis(gen) & dis(gen) & dis(gen);

    bool collision = false;
    // For each possible occupancy
    for (unsigned i = 0; i < occupancies.size(); i++) {
      // Calculate an offset into a small number of bits
      unsigned offset = (occupancies[i] * m) >> (64 - shift);
      if (table[offset] == ~0ull || table[offset] == attacks[i]) {
        // If the offset hasn't been filled already, fill it
        table[offset] = attacks[i];
      } else {
        // Otherwise there is a collision, so this is not a good magic number
        collision = true;
        break;
      }
    }

    if (!collision) {
      // This is a good magic number
      Magic mag;
      mag.magic = m;
      mag.mask = occupancy;
      mag.attack_table = table;
      mag.shift = shift;
      return mag;
    }
  }
}

void movegen_initialize_attack_boards() {
  // rook_shifts and bishop_shifts give the number of bits in the occupancy
  // relevance mask which are one. This is used for finding an efficient magic
  // number.
  int rook_shifts[64] = {
    12, 11, 11, 11, 11, 11, 11, 12,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    12, 11, 11, 11, 11, 11, 11, 12
  };
  int bishop_shifts[64] = {
    6, 5, 5, 5, 5, 5, 5, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 5, 5, 5, 5, 5, 5, 6,
  };
  for (int i = 0; i < 64; i++) {
    knight_moves[i] = 0;
    king_moves[i] = 0;
    int rank = i / 8;
    int file = i % 8;

    int nrank, nfile, square;

    // Initialize knight moves
    for (int j = -2; j <= 2; j++) {
      if (j == 0) {
        // This cannot be a legal knight move
        continue;
      }
      for (int k = -2; k <= 2; k++) {
        if (k == 0 || k == j || k == -j) {
          // This leaves only the 8 actual possiblities
          continue;
        }
        nrank = rank + j;
        nfile = file + k;
        if (nrank < 0 || nrank >= 8 || nfile < 0 || nfile >= 8) {
          // This filters moves which are off the edge of the board
          continue;
        }
        square = 8 * nrank + nfile;
        knight_moves[i] |= 1ull << square;
      }
    }

    // Initialize king moves
    for (int j = -1; j <= 1; j++) {
      for (int k = -1; k <= 1; k++) {
        if (j == 0 && k == 0) {
          continue;
        }
        nrank = rank + j;
        nfile = file + k;
        if (nrank < 0 || nrank >= 8 || nfile < 0 || nfile >= 8) {
          continue;
        }
        square = 8 * nrank + nfile;
        king_moves[i] |= 1ull << square;
      }
    }

    rook_magics[i] = generate_magic(i, rook_shifts[i], true);
    bishop_magics[i] = generate_magic(i, bishop_shifts[i], false);
  }
}

void movegen_free_magics() {
  for (int i = 0; i < 64; i++) {
    free(rook_magics[i].attack_table);
    free(bishop_magics[i].attack_table);
  }
}
