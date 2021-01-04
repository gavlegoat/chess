#include <limits>

#include "search.hpp"
#include "movegen.hpp"

double quiescence_search(GameState& gs, double alpha, double beta) {
  // TODO
}

double alpha_beta(GameState& gs, unsigned depth, double alpha, double beta) {
  if (depth == 0) {
    return quiescence_search(gs, alpha, beta);
  }
  MoveList ml = generate_moves(gs);
  if (ml.empty()) {
    if (in_check(gs.whites_move(), gs.pos())) {
      // Checkmate
      return -std::numeric_limits<double>::max();
    } else {
      // Stalemate
      return 0.0;
    }
  }
  Move& move = ml.front();
  for (const Move& m : ml) {
    gs.make_move(m);
    double score = -alpha_beta(gs, -beta, -alpha, depth - 1);
    gs.undo_move();
    if (score >= beta) {
      return beta;
    }
    if (score >= alpha) {
      alpha = score;
    }
  }
  return alpha;
}

std::pair<double, Move> BasicAlphaBetaSearcher::search(GameState& gs, unsigned depth) {
  MoveList ml = generate_moves(gs);
  double best_score = -std::numeric_limits<double>::max();
  Move best_move = Move(0, 0, 0, 0);
  for (const Move& m : ml) {
    gs.make_move(m);
    double score = alpha_beta(gs, depth, -std::numeric_limits<double>::max(),
        std::numeric_limits<double>::max());
    gs.undo_move();
    if (!gs.whites_move()) {
      score = -score;
    }
    if (score > best_score) {
      best_score = score;
      best_move = m;
    }
  }
  return std::make_pair(best_score, best_move);
}
