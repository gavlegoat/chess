#pragma once

#include "boards.hpp"

/**
 * \brief A class describing a search algorithm.
 *
 * This is implemented as a class to allow the search algorithm to maintain
 * internal state, which can often dramatically improve search times.
 */
class Searcher {
  public:
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
 * quiescence search but no other optimizations.
 */
class BasicAlphaBetaSearcher: public Searcher {
  public:
    std::pair<double, Move> search(GameState& gs, unsigned depth) override;
};
