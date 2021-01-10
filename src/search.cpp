#include <limits>
#include <queue>

#include "search.hpp"
#include "movegen.hpp"

int piece_score(int piece) {
  switch (piece) {
    case Position::W_PAWN:
    case Position::B_PAWN:
      return 1;
    case Position::W_KNIGHT:
    case Position::B_KNIGHT:
    case Position::W_BISHOP:
    case Position::B_BISHOP:
      return 3;
    case Position::W_ROOK:
    case Position::B_ROOK:
      return 5;
    case Position::W_QUEEN:
    case Position::B_QUEEN:
      return 9;
    default:
      throw std::runtime_error("Illegal capture");
  }
}

// Define an ordering on moves for the search
class MoveOrdering {
  private:
    const GameState& gs;

  public:
    MoveOrdering(const GameState& g): gs{g} {}

    bool operator()(const Move& l, const Move& r) {
      if (l.capture() && !r.capture()) {
        return true;
      }
      if (!l.capture()) {
        return false;
      }
      int lscore = piece_score(gs.pos().get_piece(l.to_square())) -
          piece_score(l.piece());
      int rscore = piece_score(gs.pos().get_piece(r.to_square())) -
          piece_score(r.piece());
      return rscore < lscore;
    }
};

double BasicAlphaBetaSearcher::alpha_beta(GameState& gs, unsigned depth,
    double alpha, double beta, bool quiescence_search) {
  if (depth == 0 && !quiescence_search) {
    return alpha_beta(gs, 0, alpha, beta, true);
  }
  MoveList move_list = generate_moves(gs);
  MoveOrdering mo(gs);
  std::priority_queue<Move, std::vector<Move>, MoveOrdering>
    ml(move_list.begin(), move_list.end(), mo);
  if (ml.empty()) {
    if (in_check(gs.whites_move(), gs.pos())) {
      // Checkmate
      return -std::numeric_limits<double>::max();
    } else {
      // Stalemate
      return 0.0;
    }
  }
  bool q_finished = true;
  while (!ml.empty()) {
    const Move& m = ml.top();
    if (quiescence_search && !m.capture()) {
      continue;
    }
    if (m.capture()) {
      q_finished = false;
    }
    gs.make_move(m);
    double score = -alpha_beta(gs, -beta, -alpha, depth - 1, quiescence_search);
    gs.undo_move();
    if (score >= beta) {
      return beta;
    }
    if (score >= alpha) {
      alpha = score;
    }
    ml.pop();
  }
  if (q_finished && quiescence_search) {
    double score = eval->evaluate_position(gs);
    if (gs.whites_move()) {
      return score;
    } else {
      return -score;
    }
  }
  return alpha;
}

Searcher::Searcher(std::unique_ptr<Evaluator>&& e): eval{std::move(e)} {}

BasicAlphaBetaSearcher::BasicAlphaBetaSearcher(std::unique_ptr<Evaluator>&& e):
  Searcher(std::move(e)) {}

std::pair<double, Move> BasicAlphaBetaSearcher::search(GameState& gs,
    unsigned depth) {
  MoveList ml = generate_moves(gs);
  double best_score = -std::numeric_limits<double>::max();
  Move best_move = Move(0, 0, 0, 0);
  for (const Move& m : ml) {
    gs.make_move(m);
    double score = alpha_beta(gs, depth, -std::numeric_limits<double>::max(),
        std::numeric_limits<double>::max(), false);
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
