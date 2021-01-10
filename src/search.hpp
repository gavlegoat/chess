#pragma once

#include <memory>

#include "boards.hpp"
#include "evaluation.hpp"

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

    virtual void initialize(GameState& gs) {};
    /**
     * \brief Find the best move and the score of that move.
     */
    virtual std::pair<double, Move> search(GameState& gs, unsigned depth) = 0;
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
    double alpha_beta(GameState& gs, unsigned depth, double alpha, double beta,
        bool quiescence_search);

  public:
    BasicAlphaBetaSearcher(std::unique_ptr<Evaluator>&& e);
    std::pair<double, Move> search(GameState& gs, unsigned depth) override;
};
