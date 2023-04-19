#pragma once

#include <memory>
#include <optional>
#include <mutex>

#include "boards.hpp"
#include "evaluation.hpp"
#include "movegen.hpp"

/**
 * \brief Information the engine shoudld send to the GUi.
 *
 * For the most part, this information can all be approximate so we don't
 * care about race conditions. The exception is the principle variation,
 * because we don't want to read half the list then have it update.
 */
struct SearchInfo {
  /** The current best score estimate of the position. */
  double score;
  /** The current search depth */
  unsigned depth;
  /** The total number of nodes searched so far. */
  unsigned nodes;
  /** The amount of time spent searching */
  unsigned time;
  /** The current principle variation */
  MoveList pv;
  /** A lock for interacting with the principle variation. */
  std::mutex pv_lock;
};

/**
 * \brief Ways for the user to limit the search.
 */
struct SearchLimits {
  /** A maximum allowed amount of time. */
  std::optional<unsigned> timeout;
  /** The maximum number of nodes to search. */
  std::optional<unsigned> node_limit;
  /** The maximum depth to search to. */
  std::optional<unsigned> depth_limit;
  /** Look for mate in N moves. */
  std::optional<unsigned> mate_in;
  /** A set of first moves to restrict the search to. */
  std::optional<MoveList> moves;
};

/**
 * \brief A class describing a search algorithm.
 *
 * This is implemented as a class to allow the search algorithm to maintain
 * internal state, which can often dramatically improve search times.
 */
class Searcher {
  protected:
    std::unique_ptr<Evaluator> eval;

  public:
    virtual ~Searcher() {}

    /**
     * Construct a new searcher with the given evaluation function.
     */
    Searcher(std::unique_ptr<Evaluator>&& e);

    /**
     * \brief Initialize the internal state of the searcher.
     */

    virtual void initialize(const GameState& gs) {};
    /**
     * \brief Find the best move and the score of that move.
     *
     * \param gs The current state of the game.
     * \param limits Limits placed on the search procedure.
     * \param info The location to write search data to during the search.
     * \param stop_signal Another thread sets this to true to end the search.
     */
    virtual std::pair<double, Move> search(GameState& gs,
        const SearchLimits& limits, SearchInfo& info, bool& stop_signal) = 0;
};

struct SearchResult {
  double score;
  MoveList pv;

  SearchResult(double s, MoveList ml):
    score{s}, pv{ml} {}
};

/**
 * \brief A basic minimax search with alpha-beta pruning.
 *
 * This is a very basic search algorithm including alpha-beta pruning and a
 * quiescence search. We also use a very simple move ordering to improve
 * pruning: for captures we will order moves by the difference between the
 * value of the captured piece and the value of the capturing piece. All other
 * moves are currently not ordered in any particular way.
 */
class BasicAlphaBetaSearcher: public Searcher {
  private:
    MoveList principle_variation;

    SearchResult alpha_beta(GameState& gs, unsigned depth,
        double alpha, double beta, bool quiescence_search, SearchInfo& info,
        bool& stop_signal, MoveList::const_iterator pv_iter,
        unsigned max_nodes);

  public:
    BasicAlphaBetaSearcher(std::unique_ptr<Evaluator>&& e);

    std::pair<double, Move> search(GameState& gs,
        const SearchLimits& limits, SearchInfo& info,
        bool& stop_signal) override;
};

