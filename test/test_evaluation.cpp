#include "catch.hpp"

#include "evaluation.hpp"

SCENARIO("evaluation is correct on some known positions") {
  GIVEN("a symmetrical position") {
    WHEN("we have the starting position") {
      GameState gs;
      THEN("the basic evaluation is 0") {
        BasicEvaluator b;
        CHECK(std::abs(b.evaluate_position(gs)) <= 0.001);
      }
    }

    WHEN("we look at another symmetrical position") {
      GameState gs("r2qk2r/ppp2ppp/2np1n2/2b1p1B1/2B1P1b1/2NP1N2/PPP2PPP/R2QK2R w KQkq - 0 1");
      THEN("the basic evaluation is 0") {
        BasicEvaluator b;
        CHECK(std::abs(b.evaluate_position(gs)) <= 0.001);
      }
    }

    WHEN("we look at the position after e4-e5") {
      GameState gs("rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 1");
      THEN("the basic evaluation is zero") {
        BasicEvaluator b;
        CHECK(std::abs(b.evaluate_position(gs)) <= 0.001);
      }
    }
  }

  GIVEN("some test positions") {
    WHEN("we evaluate the position with flipped colors") {
      GameState gs1("rnbq1rk1/pp1n1pbp/3p2p1/1BpP4/P4PP2/2N5/1P4PP/R1BQK1NR w KQkq - 0 1");
      GameState gs2("r1bqk1nr/1p4pp/2n5/p4pp2/1bPp4/3P2P1/PP1N1PBP/RNBQ1RK1 b KQkq - 0 1");
      BasicEvaluator b;
      double s1 = b.evaluate_position(gs1);
      double s2 = b.evaluate_position(gs2);
      THEN("the evaluations are opposite") {
        CHECK(std::abs(s1 + s2) <= 0.001);
      }
    }

    WHEN("we evaluate a position with a known evaluation") {
      GameState gs("rnbq1rk1/pp1n1pbp/3p2p1/1BpP4/P3PP2/2N5/1P4PP/R1BQK1NR w KQkq - 0 1");
      BasicEvaluator b;
      // Material is even (0)
      // Mobility: white - 40, black - 30 (+1)
      // Both sides have the bishop pair (0)
      // Neither side has doubled or isolated pawns (0)
      // So the final result should be +1
      THEN("the evaluation is correct") {
        CHECK(std::abs(b.evaluate_position(gs) - 1.0) <= 0.001);
      }
    }

    WHEN("we evaluate a second known position") {
      GameState gs("r1bq1rk1/pp3ppp/2n1pn2/2p5/2pP4/P1PBPN2/5PPP/R1BQ1RK1 w KQkq - 0 1");
      BasicEvaluator b;
      // Black is up a pawn (-1)
      // Mobility: white - 31, black - 34 (-0.3)
      // White has the bishop pair (+0.5)
      // Black has doubled pawns and white has an isolated pawn (0)
      // So the final score should be -0.8
      THEN("the evaluation is correct") {
        CHECK(std::abs(b.evaluate_position(gs) + 0.8) <= 0.001);
      }
    }
  }
}
