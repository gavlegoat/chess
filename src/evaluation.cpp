#include "evaluation.hpp"
#include "movegen.hpp"

double BasicEvaluator::evaluate_position(GameState& gs) {

  // Pure material considerations
  int white_queens = gs.pos().find_piece(Position::W_QUEEN).size();
  int black_queens = gs.pos().find_piece(Position::B_QUEEN).size();
  int white_rooks = gs.pos().find_piece(Position::W_ROOK).size();
  int black_rooks = gs.pos().find_piece(Position::B_ROOK).size();
  int white_bishops = gs.pos().find_piece(Position::W_BISHOP).size();
  int black_bishops = gs.pos().find_piece(Position::B_BISHOP).size();
  int white_knights = gs.pos().find_piece(Position::W_KNIGHT).size();
  int black_knights = gs.pos().find_piece(Position::B_KNIGHT).size();
  int white_pawns = gs.pos().find_piece(Position::W_PAWN).size();
  int black_pawns = gs.pos().find_piece(Position::B_PAWN).size();
  double material_score = 9.0 * (white_queens - black_queens) +
                          5.0 * (white_rooks - black_rooks) +
                          3.0 * (white_bishops - black_bishops) +
                          3.0 * (white_knights - black_knights) +
                          (white_pawns - black_pawns);

  // Mobility
  int to_move_mobility = generate_moves(gs).size();
  gs.flip_move();
  int other_mobility = generate_moves(gs).size();
  gs.flip_move();
  int white_mobility = gs.whites_move() ? to_move_mobility : other_mobility;
  int black_mobility = gs.whites_move() ? other_mobility : to_move_mobility;
  double mobility_score = 0.1 * (white_mobility - black_mobility);

  // The bishop pair is thought to be worth roughly half a pawn.
  double white_bishop_pair = white_bishops == 2 ? 1.0 : 0.0;
  double black_bishop_pair = black_bishops == 2 ? 1.0 : 0.0;
  double bishop_pair_score = 0.5 * (white_bishop_pair - black_bishop_pair);

  // Pawn structure considerations -- doubled, isolated pawns.
  const std::set<int>& w_pawns = gs.pos().find_piece(Position::W_PAWN);
  const std::set<int>& b_pawns = gs.pos().find_piece(Position::B_PAWN);

  int w_pawn_files[8];
  int b_pawn_files[8];
  for (int i = 0; i < 8; i++) {
    w_pawn_files[i] = 0;
    b_pawn_files[i] = 0;
  }
  for (int p : w_pawns) {
    w_pawn_files[p % 8]++;
  }
  for (int p : b_pawns) {
    b_pawn_files[p % 8]++;
  }
  double structure_score = 0.0;
  for (int i = 0; i < 8; i++) {
    // Check for doubled pawns
    if (w_pawn_files[i] >= 2) {
      structure_score -= 0.5;
    }
    if (b_pawn_files[i] >= 2) {
      structure_score += 0.5;
    }

    // Check for isolated pawns.
    if (w_pawn_files[i] >= 1 && (i == 0 || w_pawn_files[i-1] == 0) &&
        (i == 7 || w_pawn_files[i+1] == 0)) {
        structure_score -= 0.5;
    }
    if (b_pawn_files[i] >= 1 && (i == 0 || b_pawn_files[i-1] == 0) &&
        (i == 7 || b_pawn_files[i+1] == 0)) {
        structure_score += 0.5;
    }
  }

  return material_score + mobility_score + bishop_pair_score + structure_score;
}
