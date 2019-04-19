#include <string>

#include "boards.hpp"
#include "utils.hpp"

Position::Position() {
  for (int i = 0; i < NUM_BOARDS; i++) {
    boards[i] = 0;
  }
}

Position::Position(std::string fen) {
  for (int i = 0; i < NUM_BOARDS; i++) {
    boards[i] = 0;
  }
  std::vector<std::string> ranks = split(fen, '/');
  for (int i = 0; i < 8; i++) {
    int j = 0;
    int file = 0;
    while (file < 8) {
      int pos = (7 - i) * 8 + file;
      switch (ranks[i][j]) {
        case 'P':
          place_piece(pos, W_PAWN);
          file++;
          break;
        case 'N':
          place_piece(pos, W_KNIGHT);
          file++;
          break;
        case 'B':
          place_piece(pos, W_BISHOP);
          file++;
          break;
        case 'R':
          place_piece(pos, W_ROOK);
          file++;
          break;
        case 'K':
          place_piece(pos, W_KING);
          file++;
          break;
        case 'Q':
          place_piece(pos, W_QUEEN);
          file++;
          break;
        case 'p':
          place_piece(pos, B_PAWN);
          file++;
          break;
        case 'n':
          place_piece(pos, B_KNIGHT);
          file++;
          break;
        case 'b':
          place_piece(pos, B_BISHOP);
          file++;
          break;
        case 'r':
          place_piece(pos, B_ROOK);
          file++;
          break;
        case 'k':
          place_piece(pos, B_KING);
          file++;
          break;
        case 'q':
          place_piece(pos, B_QUEEN);
          file++;
          break;
        default:
          file += ranks[i][j] - '0';
      }
      j++;
    }
  }
}

void Position::make_move(const Move& m) {
  int from_square = m.from_square();
  int to_square = m.to_square();
  int piece = m.piece();
  if (m.capture()) {
    // Determine which square the captured piece was on. This is just the "to"
    // square unless the capture was en passant.
    int captured_square = to_square;
    if (m.capture_ep()) {
      if (piece == W_PAWN) {
        captured_square -= 8;
      } else {
        captured_square += 8;
      }
    }
    // Clear the captured square on each board
    for (int i = 0; i < NUM_BOARDS; i++) {
      remove_piece(captured_square, i);
    }
  }
  // Remove the relevant piece from the "from" square
  remove_piece(from_square, piece);

  // If this is a promotion, place the appropriate piece on the "to" square
  if (m.promote_knight()) {
    if (piece == W_PAWN) {
      place_piece(to_square, W_KNIGHT);
    } else {
      place_piece(to_square, B_KNIGHT);
    }
  } else if (m.promote_bishop()) {
    if (piece == W_PAWN) {
      place_piece(to_square, W_BISHOP);
    } else {
      place_piece(to_square, B_BISHOP);
    }
  } else if (m.promote_rook()) {
    if (piece == W_PAWN) {
      place_piece(to_square, W_ROOK);
    } else {
      place_piece(to_square, B_ROOK);
    }
  } else if (m.promote_queen()) {
    if (piece == W_PAWN) {
      place_piece(to_square, W_QUEEN);
    } else {
      place_piece(to_square, B_QUEEN);
    }
  } else {
    // This move is not a promotion so we just place the piece we had
    place_piece(to_square, piece);
  }

  // Handle castling
  if (m.castle_queenside()) {
    if (piece == W_KING) {
      remove_piece(0, W_ROOK);
      place_piece(3, W_ROOK);
    } else {
      remove_piece(56, B_ROOK);
      place_piece(59, B_ROOK);
    }
  } else if (m.castle_kingside()) {
    if (piece == W_KING) {
      remove_piece(7, W_ROOK);
      place_piece(5, W_ROOK);
    } else {
      remove_piece(63, B_ROOK);
      place_piece(61, B_ROOK);
    }
  }
}

std::string Position::fen_board() const {
  std::string ret = "";
  for (int i = 0; i < 8; i++) {
    int empty_counter = 0;
    for (int j = 0; j < 8; j++) {
      int square = (7 - i) * 8 + j;
      bool empty = false;
      char code = ' ';
      if (piece_at(square, W_PAWN)) {
        code = 'P';
      } else if (piece_at(square, W_KNIGHT)) {
        code = 'N';
      } else if (piece_at(square, W_BISHOP)) {
        code = 'B';
      } else if (piece_at(square, W_ROOK)) {
        code = 'R';
      } else if (piece_at(square, W_QUEEN)) {
        code = 'Q';
      } else if (piece_at(square, W_KING)) {
        code = 'K';
      } else if (piece_at(square, B_PAWN)) {
        code = 'p';
      } else if (piece_at(square, B_KNIGHT)) {
        code = 'n';
      } else if (piece_at(square, B_BISHOP)) {
        code = 'b';
      } else if (piece_at(square, B_ROOK)) {
        code = 'r';
      } else if (piece_at(square, B_QUEEN)) {
        code = 'q';
      } else if (piece_at(square, B_KING)) {
        code = 'k';
      } else {
        empty = true;
        empty_counter++;
      }
      if (!empty) {
        if (empty_counter > 0) {
          ret += std::to_string(empty_counter);
        }
        ret += std::string(1, code);
        empty_counter = 0;
      }
    }
    if (empty_counter > 0) {
      ret += std::to_string(empty_counter);
    }
    if (i < 7) {
      ret += "/";
    }
  }

  return ret;
}

bool Position::operator<(const Position& other) const {
  for (int i = 0; i < NUM_BOARDS; i++) {
    if (boards[i] < other.boards[i]) {
      return true;
    }
  }
  return false;
}

GameState::GameState() :
  position(Position("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR")),
  white_to_move(true), w_castle_k(true), w_castle_q(true), b_castle_k(true),
  b_castle_q(true), en_passant_square(0x0000000000000000ull),
  en_passant_possible(false), half_moves_since_reset(0), moves(1),
  repeats(std::map<Position, int>()) {}

GameState::GameState(std::string fen) {
  std::vector<std::string> words = split(fen, ' ');
  std::string board = words[0];
  std::string color = words[1];
  std::string castling = words[2];
  std::string en_passant = words[3];
  std::string half_moves = words[4];
  std::string move_number = words[5];

  white_to_move = (color == "w");
  w_castle_k = (castling.find("K") != std::string::npos);
  w_castle_q = (castling.find("Q") != std::string::npos);
  b_castle_k = (castling.find("k") != std::string::npos);
  b_castle_q = (castling.find("q") != std::string::npos);
  if (en_passant != "-") {
    en_passant_square = algebraic_to_int(en_passant);
    en_passant_possible = true;
  } else {
    en_passant_possible = false;
  }
  half_moves_since_reset = std::stoi(half_moves);
  repeats = std::map<Position, int>();
  position = Position(board);
  moves = std::stoi(move_number);
}

GameState::GameState(Position pos, bool wtm, bool wck, bool wcq, bool bck,
    bool bcq, uint64_t eps, bool epp, int msr, int ms,
    std::map<Position, int> rs) :
  position(pos), white_to_move(wtm), w_castle_k(wck), w_castle_q(wcq),
  b_castle_k(bck), b_castle_q(bcq), en_passant_square(eps),
  en_passant_possible(epp), half_moves_since_reset(msr), moves(ms),
  repeats(rs) {}

void GameState::make_move(const Move& m) {
  // Change the current board state
  position.make_move(m);
  // Update castling possiblities
  // Since both players often castle early in the game we wrap this in a check
  // to see if the current player can castle in order to skip it in most runs
  if (( white_to_move && (w_castle_q || w_castle_k)) ||
      (!white_to_move && (b_castle_q || b_castle_k))) {
    if (m.castle_queenside()) {
      if (white_to_move) {
        w_castle_q = false;
      } else {
        b_castle_q = false;
      }
    } else if (m.castle_kingside()) {
      if (white_to_move) {
        w_castle_k = false;
      } else {
        b_castle_k = false;
      }
    }

    // If the king or rook moved, update castling possibilities
    int from_square = m.from_square();
    int to_square = m.to_square();
    if (white_to_move) {
      if (position.piece_at(to_square, Position::W_KING)) {
        w_castle_q = false;
        w_castle_k = false;
      } else if (position.piece_at(to_square, Position::W_ROOK)) {
        if (from_square == 0) {
          // This is white's queenside rook
          w_castle_q = false;
        } else {
          w_castle_k = false;
        }
      }
    } else {
      if (position.piece_at(to_square, Position::B_KING)) {
        b_castle_q = false;
        b_castle_k = false;
      } else if (position.piece_at(to_square, Position::B_ROOK)) {
        if (from_square == 56) {
          // This is black's queenside rook
          b_castle_q = false;
        } else {
          b_castle_k = false;
        }
      }
    }
  }

  // Update en passant possibilities
  if (m.double_pawn_push()) {
    en_passant_possible = true;
    if (white_to_move) {
      // White moved so the relevant square is behind the new square
      en_passant_square = m.to_square() - 8;
    } else {
      en_passant_square = m.to_square() + 8;
    }
  } else {
    en_passant_possible = false;
  }
  // Update the 50-move counter
  if (m.double_pawn_push() || m.capture() ||
      m.piece() == Position::W_PAWN || m.piece() == Position::B_PAWN) {
    half_moves_since_reset = 0;
  } else {
    half_moves_since_reset++;
  }
  // Update the move counter
  if (!white_to_move) {
    moves++;
  }
  // Update repeated positions
  if (repeats.find(position) == repeats.end()) {
    repeats[position] = 1;
  } else {
    repeats[position]++;
  }
  // It's now the next player's turn
  white_to_move = !white_to_move;
}

// Convert a GameState to a FEN string
std::string GameState::fen_string() const {
  std::string ret = position.fen_board();
  ret += " ";
  if (white_to_move) {
    ret += "w ";
  } else {
    ret += "b ";
  }
  std::string castle = "";
  if (w_castle_k) {
    castle += "K";
  }
  if (w_castle_q) {
    castle += "Q";
  }
  if (b_castle_k) {
    castle += "k";
  }
  if (b_castle_q) {
    castle += "q";
  }
  if (castle == "") {
    ret += "- ";
  } else {
    ret += castle + " ";
  }
  if (en_passant_possible) {
    ret += int_to_algebraic(en_passant_square) + " ";
  } else {
    ret += "- ";
  }
  ret += std::to_string(half_moves_since_reset) + " ";
  ret += std::to_string(moves);
  return ret;
}

std::ostream& operator<<(std::ostream& os, const GameState& gs) {
  os << gs.fen_string();
}

Move::Move(int from, int to, int p, uint16_t fl) :
  from_sq(from), to_sq(to), piece_moved(p), flags(fl) {}
