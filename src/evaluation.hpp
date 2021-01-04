#pragma once

#include "boards.hpp"

/**
 * \brief A class for evaluating positions.
 *
 * An evaluator holds a function which can evaluate positions to see how likely
 * each side is to win. It is implemented as a class here to allow evaluation
 * strategies to keep internal state, since that can often speed up evaluations
 * on a sequence of similar positions. Note that checkmate is expected to be
 * handled separately by the search.
 */
class Evaluator {
  public:
    /**
     * \brief Initialize any internal state needed by this evaluator.
     */
    virtual void initialize(GameState& gs) {}

    /**
     * \brief Guess how likely each side is to win.
     *
     * The evaluation is normalized to the value of a pawn, with positive
     * numbers being good for white and negative numbers being good for black.
     * For example, if black has an extra knight and the position is otherwise
     * equal, the score would be -3.
     */
    virtual double evaluate_position(GameState& gs) = 0;
};

/**
 * \brief A very simple evaluation function.
 *
 * This class uses a very basic evaluation function with no performance
 * improvements. It is meant for testing and getting off the ground and
 * will not give very good performance.
 */
class BasicEvaluator: public Evaluator {
  public:
    double evaluate_position(GameState& gs) override;
};
