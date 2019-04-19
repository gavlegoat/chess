#include "movegen.hpp"

#include <cstdlib>
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
//
// Given a Magic object m and a piece occupancy bitboard occ for the whole
// board, we compute the possible attacks as
// attack_table[((occ & m.mask) * magic) >> (64 - shift)]
typedef struct {
  uint64_t magic;
  uint64_t mask;
  uint64_t* attack_table;
  int shift;
} Magic;

// These variables hold masks showing where each piece can potentially move
// given an empty board.
uint64_t knight_moves[64];
uint64_t king_moves[64];

Magic rook_magics[64];
Magic bishop_magics[64];

// Generate a bitboard of all checking pieces
uint64_t get_check_board(const GameState& gs) {
  bool white_to_move = gs.whites_move();
  Positionp = gs.pos();
  int king = Position::color_piece(Position::KING, white_to_move);
  int king_sq = *find_piece[king].begin();
  int opp_knight = Position::color_piece(Position::KNIGHT, !white_to_move);
  int opp_bishop = Position::color_piece(Position::BISHOP, !white_to_move);
  int opp_rook = Position::color_piece(Position::ROOK, !white_to_move);
  int opp_queen = Position::color_piece(Position::QUEEN, !white_to_move);
  uint64_t knight_board = p.get_board(opp_knight);
  uint64_t bishop_board = p.get_board(opp_bishop);
  uint64_t rook_board = p.get_board(opp_rook);
  uint64_t queen_board = p.get_board(opp_queen);
  Magic bm = bishop_magics[king_sq];
  uint64_t occupancy = p.get_board(Position::BOTH_ALL);
  uint64_t bishop_attack = bm.attack_table[((occupancy & bm.mask) * bm.magic)
    >> (64 - bm.shift)];
  uint64_t rook_attack = rm.attack_table[((occupancy & rm.mask) & rm.magic)
    >> (64 - rm.shift)];
  uint64_t check_board = knight_moves[king_sq] & opp_knight;
  check_board |= bishop_attack & (bishop_board | queen_board);
  check_board |= rook_attack & (rook_board | queen_board);

  int opp_pawn = Position::color_piece(Position::PAWN, !white_to_move);
  int krank = king_sq / 8;
  int kfile = king_sq % 8;
  if (kfile < 7) {
    if (white_to_move && krank < 7) {
      if (p.piece_at(king_sq + 9, opp_pawn)) {
        check_board |= 1ull << (king_sq + 9);
      }
    } else if (krank > 0) {
      if (p.piece_at(king_sq - 7, opp_pawn)) {
        check_board |= 1ull << (king_sq - 7);
      }
    }
  }
  if (kfile > 0) {
    if (white_to_move && krank < 7) {
      if (p.piece_at(king_sq + 7, opp_pawn)) {
        check_board |= 1ull << (king_sq + 7);
      }
    } else if (krank > 0) {
      if (p.piece_at(king_sq - 9, opp_pawn)) {
        check_board |= 1ull << (king_sq - 9);
      }
    }
  }
  return check_board;
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
#error "Currently only compilers defining __GNUC__ are supported"
#endif

// Append all of the moves for some piece type which can end on the given
// square.
void append_moves_to(uint64_t from_squares, int to_square,
    int piece, bool capture, MoveList& l) {
  while (from_squares != 0) {
    int fsq = lsb(from_squares);
    from_squares &= from_squares - 1;
    if (capture) {
      l.push_back(Move(fsq, to_square, piece, Move::CAPTURE));
    } else {
      l.push_back(Move(fsq, to_square, piece, Move::QUIET));
    }
  }
}

// Append all of the moves for a given piece type starting from some square.
void append_moves_from(int from_square, uint64_t to_squares,
    int piece, uint64_t opp_pieces, MoveList& l) {
  while (to_squares != 0) {
    int tsq = lsb(to_squares);
    to_squares &= to_squares - 1;
    if (opp_pieces & ~(1ull << tsq) != 0) {
      l.push_back(Move(from_square, tsq, piece, Move::CAPTURE);
    } else {
      l.push_back(Move(from_square, tsq, piece, Move::QUIET);
    }
  }
}

// Generate pseudolegal king moves
MoveList generate_king_moves(const GameState& gs) {
  Position p = gs.pos();
  bool white_to_move = gs.whites_move();
  int our_king = Position::color_piece(Position::KING, white_to_move);
  int our_pieces = Position::color_piece(Position::ALL, white_to_move);
  int king_square = *p.find_piece(our_king).begin();
  uint64_t king_move_board = king_moves[king_square];
  king_move_board &= ~pos.get_board(our_pieces);
  int opp_pieces = Position::color_piece(Position::ALL, !white_to_move);
  uint64_t opp_all_board = p.get_board(opp_pieces);
  MoveList ret;
  append_moves_from(king_square, king_move_board, our_king, opp_all_board, ret);
  return ret;
}

void append_castling_moves(const GameState& gs, MoveList& l) {
  // TODO
}

void append_en_passant(const GameState& gs, MoveList& l) {
  if (!gs.en_passant()) {
    return;
  }
  int square = gs.en_passant_target();
  bool white_to_move = gs.whites_move();
  Position p = gs.pos();
  if (white_to_move) {
    if (p.piece_at(square - 9, our_pawn)) {
      l.push_back(Move(square - 9, square, our_pawn, Move::CAPTURE_EP));
    }
    if (p.peice_at(square - 7, our_pawn)) {
      l.push_back(Move(square - 7, square, our_pawn, Move::CAPTURE_EP));
    }
  } else {
    if (p.piece_at(square + 9, our_pawn)) {
      l.push_back(Move(square + 9, square, our_pawn, Move::CAPTURE_EP));
    }
    if (p.peice_at(square + 7, our_pawn)) {
      l.push_back(Move(square + 7, square, our_pawn, Move::CAPTURE_EP));
    }
  }
}

// Generate the pseudo-legal moves in this position. We will need to filter out
// moves that result in check later.
MoveList generate_pseudolegal_moves(const GameState& gs) {
  // We'll generate non-castling king moves first. We will always need to
  // consider king moves so we might as well do it now.
  MoveList ret = generate_king_moves(gs);
  // Next we'll see if we're in check. We do this early because if we are in
  // check then we have a very limited set of moves to choose from so we can
  // avoid generating a lot of illegal moves.
  uint64_t check_board = get_check_board(gs);
  int count = popcount(check_board);
  Position p = gs.pos();
  bool white_to_move = gs.whites_move();
  if (count < 1) {
    // We are not in check, so generate all moves
    append_castling_moves(gs, ret);
    append_en_passant(gs, ret);

    // TODO: Generate all normal moves

    // For knights, bishops, rooks, and queens, the only restriction on moving is
    // if the piece is pinned
  } else if (count < 2) {
    // We are checked by one piece, so we only need to look for moves which
    // capture the piece or block the check (for a sliding attacker)
    int checking_piece_sq = lsb(check_board);
    // Calculate a set of legal target squares
    uint64_t legal_targets = 1ull << checking_piece_sq;
    int our_king = Position::color_piece(Position::KING, white_to_move);
    int opp_knight = Position::color_piece(Position::KNIGHT, !white_to_move);
    int our_knight = Position::color_piece(Position::KNIGHT, white_to_move);
    int our_bishop = Position::color_piece(Position::BISHOP, white_to_move);
    int our_rook = Position::color_piece(Position::ROOK, white_to_move);
    int our_queen = Position::color_piece(Position::QUEEN, white_to_move);
    int our_pawn = Position::color_piece(Position::PAWN, white_to_move);
    int opp_pieces = Position::color_piece(Position::ALL, !white_to_move);
    // If the checking piece is a kngiht then the only legal target square is
    // the position of the checking piece, so there's nothing to do
    if (!piece_at(checking_piece_sq, opp_knight)) {
      // Otherwise, the piece is a sliding attacker, so the legal squares
      // consist of the location of the attacking piece along with a line
      // connecting the attacking piece to our king. Note that the same applies
      // to pawns even thought they aren't technically sliding attackers.
      int our_king_pos = *p.find_piece(our_king).begin();
      int rank = checking_piece_sq / 8;
      int file = checking_piece_sq % 8;
      int king_rank = our_king_pos / 8;
      int king_file = our_king_pos % 8;
      int drank = rank > king_rank ? -1 : (rank < king_rank ? 1 : 0);
      int dfile = file > king_file ? -1 : (file < king_file ? 1 : 0);
      while (rank != king_rank || file != king_file) {
        legal_targets |= 1ull << (rank * 8 + file);
        rank += drank;
        file += dfile;
      }
    }
    // Generate moves that end on legal_targets
    uint64_t occupancy = p.get_board(Position::BOTH_ALL);
    while (legal_targets != 0) {
      int square = lsb(legal_targets);
      legal_targets &= legal_targets - 1;
      uint64_t knights = knight_moves[square] & p.get_board(our_knight);
      bool capture = p.get_board(opp_pieces & ~(1ull << square) != 0;
      append_moves_to(knights, square, our_knight, capture, ret);
      Magic rm = rook_magic[square];
      Magic bm = bishop_magic[square];
      uint64_t r_att = rm.attack_table[(occupancy * rm.magic) << (64 - rm.shift)];
      uint64_t b_att = bm.attack_table[(occupancy * bm.magic) << (64 - bm.shift)];
      uint64_t rooks = r_att & p.get_board(our_rook);
      uint64_t bishops = b_att & p.get_board(our_bishop);
      uint64_t queens = (r_att | b_att) & p.get_board(our_queen);
      append_moves_to(rooks, square, our_rook, capture, ret);
      append_moves_to(bishops, square, our_bishop, capture, ret);
      append_moves_to(queens, square, our_queen, capture, ret);
      std::set<int> pawn_squares = pos.find_piece(our_pawn);
      bool promote = white_to_move ? square / 8 == 7 : square / 8 == 0;
      for (int pawn : pawn_squares) {
        if (capture) {
          if (( white_to_move && (pawn == square - 7 || pawn == square - 9)) ||
              (!white_to_move && (pawn == square + 7 || pawn == square + 9))) {
            if (promote) {
              ret.push_back(Move(pawn, square, our_pawn, Move::PROMOTE_KNIGHT_CAPTURE));
              ret.push_back(Move(pawn, square, our_pawn, Move::PROMOTE_BISHOP_CAPTURE));
              ret.push_back(Move(pawn, square, our_pawn, Move::PROMOTE_ROOK_CAPTURE));
              ret.push_back(Move(pawn, square, our_pawn, Move::PROMOTE_QUEEN_CAPTURE));
            } else {
              ret.push_back(Move(pawn, square, our_pawn, Move::CAPTURE));
            }
          } else if (gs.en_passant() &&
                     (( white_to_move && gs.en_passant_square() == square + 8) ||
                      (!white_to_move && gs.en_passant_square() == square - 8)) &&
                     (pawn == square - 1 || pawn == square + 1)) {
            ret.push_back(Move(pawn, gs.en_passant_square(), our_pawn, Move::CAPTURE_EP));
          }
        } else {
          if (( white_to_move && pawn == square - 8) ||
              (!white_to_move && pawn == square + 8)) {
            if (promote) {
              ret.push_back(Move(pawn, square, our_pawn, Move::PROMOTE_KNIGHT));
              ret.push_back(Move(pawn, square, our_pawn, Move::PROMOTE_BISHOP));
              ret.push_back(Move(pawn, square, our_pawn, Move::PROMOTE_ROOK));
              ret.push_back(Move(pawn, square, our_pawn, Move::PROMOTE_QUEEN));
            } else {
              ret.push_back(Move(pawn, square, our_pawn, Move::QUIET));
            }
          } else if (( white_to_move && pawn / 8 == 1 && pawn == square - 16) ||
                     (!white_to_move && pawn / 8 == 6 && pawn == square + 16)) {
            ret.push_back(Move(pawn, square, our_pawn, Move::PAWN_DOUBLE));
          } else if (gs.en_passant() && gs.en_passant_square() == square &&
                     (( white_to_move && (pawn == square - 7 || pawn == square - 9)) ||
                      (!white_to_move && (pawn == square + 7 || pawn == square + 9)))) {
            ret.push_back(Move(pawn, square, our_pawn, Move::CAPTURE_EP));
          }
        }
      }
    }
  }

  return ret;
}

// Check whether a given pseudolegal move results in check
bool is_legal(const Move& m, const GameState& gs) {
  // TODO
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
    return std::vector<uint64_t>();
  }
  if (occupancy & (1ull << ind) == 0) {
    return std::vector<uint64_t>();
  }
  std::vector<uint64_t> zero = generate_possible_occupancies(sq, occupancy,
      partial, ind + 1);
  std::vector<uint64_t> one = generate_possible_occupancies(sq, occupancy,
      partial | (1ull << ind), ind + 1);

  zero.insert(zero.end(), one.begin(), one.end());
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
      if (~(1ull << square) & occupancy != 0) {
        break;
      }
      nrank++;
    }
    nrank = rank - 1;
    while (nrank >= 0) {
      square = 8 * nrank + nfile;
      attack |= 1ull << square;
      nrank--;
      if (~(1ull << square) & occupancy != 0) {
        break;
      }
    }
    nrank = rank;
    nfile = file + 1;
    while (nfile < 8) {
      square = 8 * nrank + nfile;
      attack |= 1ull << square;
      nfile++;
      if (~(1ull << square) & occupancy != 0) {
        break;
      }
    }
    nfile = file - 1;
    while (nfile >= 0) {
      square = 8 * nrank + nfile;
      attack |= 1ull << square;
      nfile--;
      if (~(1ull << square) & occupancy != 0) {
        break;
      }
    }
  } else {
    nrank = rank + 1;
    nfile = file + 1;
    while (nfile < 8 && nrank < 8) {
      square = 8 * nrank + nfile;
      occupancy |= 1ull << square;
      nrank++;
      nfile++;
      if (~(1ull << square) & occupancy != 0) {
        break;
      }
    }
    nrank = rank + 1;
    nfile = file - 1;
    while (nfile >= 0 && nrank < 8) {
      square = 8 * nrank + nfile;
      occupancy |= 1ull << square;
      nrank++;
      nfile--;
      if (~(1ull << square) & occupancy != 0) {
        break;
      }
    }
    nrank = rank - 1;
    nfile = file + 1;
    while (nfile < 8 && nrank >= 0) {
      square = 8 * nrank + nfile;
      occupancy |= 1ull << square;
      nrank--;
      nfile++;
      if (~(1ull << square) & occupancy != 0) {
        break;
      }
    }
    nrank = rank - 1;
    nfile = file - 1;
    while (nfile >= 0 && nrank >= 0) {
      square = 8 * nrank + nfile;
      occupancy |= 1ull << square;
      nrank--;
      nfile--;
      if (~(1ull << square) & occupancy != 0) {
        break;
      }
    }
  }
  return attack;
}

// Generate a magic number for some square
Magic generate_magic(int square, int shift, bool is_rook) {
  uint64_t occupancy = generate_occupancy_mask(square, is_rook);
  std::vector<uint64_t> occupancies = generate_possible_occupancies(square,
      occupancy, is_rook, 0, 0);
  std::vector<uint64_t> attacks;
  for (int i = 0; i < occupancies.size(); i++) {
    attacks.push_back(generate_attack(sq, occupancies[i], is_rook));
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
    for (int i = 0; i < occupancies.size(); i++) {
      // Calculate an offset into a small number of bits
      int offset = (occupancies[i] * m) >> (64 - shift);
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
      if (j == 0) {
        continue;
      }
      for (int k = -1; k <= 1; k++) {
        if (k == 0) {
          continue;
        }
        nrank = rank + j;
        nfile = rank + k;
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
