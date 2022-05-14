#include <limits>
#include <queue>

#include "search.hpp"
#include "movegen.hpp"

/**
 * \brief Get the point value of a piece
 */
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

/**
 * \brief An ordering for moves in a search.
 *
 * Better move ordering can dramatically increase the amount of pruning that's
 * possible in the search. This class is designed to be used with a
 * std::priority_queue for move ordering. Note that this class is designed to
 * work for a specific game state, and should be reinitialized at each search
 * node.
 */
class MoveOrdering {
  private:
    /** The current state of the game. */
    const GameState& gs;
    /** The move for this position in the principle variation. */
    const std::optional<Move> pv_move;

  public:
    MoveOrdering(const GameState& g, const std::optional<Move> m):
      gs{g}, pv_move{m} {}

    /**
     * \brief Compare two moves to determine their order in the queue.
     *
     * The semantics here are that this function should return true if
     * r should be given higher priority than l.
     */
    bool operator()(const Move& l, const Move& r) {
      // The move in the principle variation is always given highest priority.
      if (pv_move && l == *pv_move) {
        return false;
      }
      if (pv_move && r == *pv_move) {
        return true;
      }
      // After that, we always search captures first
      if (l.capture() && !r.capture()) {
        return false;
      }
      if (!l.capture() && r.capture()) {
        return true;
      }
      // If both moves are captures, we consider whichever one has the larger
      // difference between the value of the capturing piece and the value of
      // the captured piece.
      if (l.capture()) {
        int lscore = piece_score(gs.pos().get_piece(l.to_square())) -
            piece_score(l.piece());
        int rscore = piece_score(gs.pos().get_piece(r.to_square())) -
            piece_score(r.piece());
        return lscore < rscore;
      }
      // If neither move is a capture, we use any arbitrary ordering.
      return true;
    }
};

/**
 * \brief Perform an alpha-beta search and get the score.
 *
 * \param gs The current state of the game.
 * \param depth The depth to search to.
 * \param alpha The current best score for the alpha-player.
 * \param beta The current best score for the beta-player.
 * \param quiescence_search True if the search is in the quiescence phase.
 * \param info An object to write search data into for passing to the GUI.
 * \param stop_signal If true, return immediately.
 * \param pv_iter The next move in the principle variation.
 * \return The value of the current position.
 */
SearchResult BasicAlphaBetaSearcher::alpha_beta(GameState& gs,
    unsigned depth, double alpha, double beta, bool quiescence_search,
    SearchInfo& info, bool& stop_signal, MoveList::const_iterator pv_iter,
    unsigned max_nodes) {
  // If we have been told to stop, return immediately.
  if (stop_signal) {
    return SearchResult(0.0, MoveList());
  }
  info.nodes++;
  if (info.nodes > max_nodes) {
    return SearchResult(0.0, MoveList());
  }
  MoveList::const_iterator next_pv_iter;
  if (pv_iter == this->principle_variation.cend()) {
    next_pv_iter = pv_iter;
  } else {
    next_pv_iter = pv_iter + 1;
  }
  // If we reach the depth limit, switch to a quiescence search
  if (depth == 0 && !quiescence_search) {
    // TODO: For now, we don't do any quiescence search
    double v = eval->evaluate_position(gs);
    if (gs.whites_move()) {
      return SearchResult(v, MoveList());
    } else {
      return SearchResult(-v, MoveList());
    }
  }
  // Generate and sort a list of possible moves
  MoveList move_list = generate_moves(gs);
  std::optional<Move> pv_move;
  if (pv_iter != this->principle_variation.cend()) {
    pv_move = *pv_iter;
  }
  MoveOrdering mo(gs, pv_move);
  std::priority_queue<Move, std::vector<Move>, MoveOrdering>
    ml(move_list.begin(), move_list.end(), mo);
  if (ml.empty()) {
    if (in_check(gs.whites_move(), gs.pos())) {
      // Checkmate
      return SearchResult(-1000.0, MoveList());
    } else {
      // Stalemate
      return SearchResult(0.0, MoveList());
    }
  }
  bool q_finished = true;   // True when the quiescence search is finished
  MoveList pv;
  while (!ml.empty()) {
    const Move& m = ml.top();
    if (m != *next_pv_iter) {
      // We are moving off the pv, so we should make sure not to use it for
      // future move ordering
      next_pv_iter = this->principle_variation.cend();
    }
    if (quiescence_search && !m.capture()) {
      continue;
    }
    if (m.capture()) {
      q_finished = false;
    }
    gs.make_move(m);
    auto res = alpha_beta(gs, depth - 1, -beta, -alpha, quiescence_search,
        info, stop_signal, next_pv_iter, max_nodes);
    gs.undo_move();
    // res.score has the opponent's perspective, so we flip it for the current
    // frame.
    double score = -res.score;
    if (score >= beta) {
      // Cutoff the search
      // We can return an empty move list here because it will not be in the
      // principle variation in the next call up the stack anyway
      return SearchResult(beta, MoveList());
    }
    if (score > alpha) {
      // New best score -- update alpha
      alpha = score;
      pv = res.pv;
      pv.push_front(m);
    }
    ml.pop();
  }
  // If the quiescence search had no more moves to consider, return the value
  // of this position.
  if (q_finished && quiescence_search) {
    double score = eval->evaluate_position(gs);
    if (gs.whites_move()) {
      return SearchResult(score, MoveList());
    } else {
      return SearchResult(-score, MoveList());
    }
  }
  return SearchResult(alpha, pv);
}

Searcher::Searcher(std::unique_ptr<Evaluator>&& e): eval{std::move(e)} {}

BasicAlphaBetaSearcher::BasicAlphaBetaSearcher(std::unique_ptr<Evaluator>&& e):
  Searcher(std::move(e)), principle_variation{} {}

std::pair<double, Move> BasicAlphaBetaSearcher::search(GameState& gs,
    const SearchLimits& limits, SearchInfo& info, bool& stop_signal) {
  MoveList ml;
  if (limits.moves) {
    // In this case the GUI has told us to only search some moves.
    ml = *limits.moves;
  } else {
    ml = generate_moves(gs);
  }
  info.nodes = 0;
  info.depth = 0;
  unsigned max_depth = limits.depth_limit.value_or(1000);
  if (limits.mate_in) {
    // mate_in is in moves, so we convert it to plies
    max_depth = 2 * (*limits.mate_in);
  }
  unsigned max_nodes = limits.node_limit.value_or(
      std::numeric_limits<unsigned>::max());
  double outer_best_score = -std::numeric_limits<double>::max();
  Move outer_best_move;
  // Outer loop for iterative deepening
  for (unsigned depth = 0; depth < max_depth; depth++) {
    double best_score = -std::numeric_limits<double>::max();
    Move best_move = Move(0, 0, 0, 0);
    MoveList best_pv;
    // Get a pointer to the head of the principle variation
    MoveList::const_iterator pv_iter = this->principle_variation.cbegin();
    MoveList::const_iterator next_pv_iter;
    if (pv_iter == this->principle_variation.cend()) {
      next_pv_iter = pv_iter;
    } else {
      next_pv_iter = pv_iter + 1;
    }
    // Create an ordered set of moves to search
    MoveOrdering mo(gs, *pv_iter);
    std::priority_queue<Move, std::vector<Move>, MoveOrdering>
      queue(ml.begin(), ml.end(), mo);
    while (!queue.empty()) {
      Move m = queue.top();
      queue.pop();
      gs.make_move(m);
      auto res = alpha_beta(gs, depth, -std::numeric_limits<double>::max(),
          -best_score, false, info, stop_signal, next_pv_iter, max_nodes);
      gs.undo_move();
      // We invert the score here because we made a move before calling into
      // alpha_beta
      double score = -res.score;
      if (score > best_score) {
        best_score = score;
        best_move = m;
        best_pv = res.pv;
        best_pv.push_front(m);
      }
      if (score > outer_best_score) {
        outer_best_score = best_score;
        outer_best_move = best_move;
        this->principle_variation = best_pv;
        info.pv_lock.lock();
        info.pv = best_pv;
        info.pv_lock.unlock();
        info.score = best_score;
      }
    }
    if (!stop_signal) {
      outer_best_score = best_score;
      outer_best_move = best_move;
      this->principle_variation = best_pv;
      info.pv_lock.lock();
      info.pv = best_pv;
      info.pv_lock.unlock();
      info.depth = depth + 1;
      info.score = outer_best_score;
    }
  }
  if (!gs.whites_move()) {
    outer_best_score = -outer_best_score;
  }
  return std::make_pair(outer_best_score, outer_best_move);
}
