#include "catch.hpp"

#include <memory>

#include "search.hpp"

SCENARIO("search is correct on some known positions") {
  GIVEN("The starting position") {
    GameState gs;
    BasicAlphaBetaSearcher searcher(std::make_unique<BasicEvaluator>());
    SearchLimits limits;
    SearchInfo info;
    bool stop_signal = false;
    WHEN("We evaluate the position to depth 2") {
      Move e4 = gs.convert_move("e2e4");
      Move e3 = gs.convert_move("e2e3");
      gs.make_move(e4);
      Move e5 = gs.convert_move("e7e5");
      Move e6 = gs.convert_move("e7e6");
      gs.undo_move();
      limits.depth_limit = 2;
      auto res = searcher.search(gs, limits, info, stop_signal);
      // At depth two, it is not possible for White to force any material
      // gain or do anything complex structurally. The best move should be e4
      // or e3 because it maximizes mobility, and the best response should be
      // e5 or e6 for the same reason. Therefore, we end up in a symmetrical
      // position and the best possible score should be zero.
      THEN("The best move is e3 or e4") {
        CHECK((res.second == e4 || res.second == e3));
      }
      THEN("The best move has evaluation zero") {
        CHECK(std::abs(res.first) <= 0.001);
      }
      THEN("The principle variation in the searcher is {e3,e4}-{e5,e6}") {
        CHECK(info.pv.size() == 2);
        CHECK((info.pv[0] == e4 || info.pv[0] == e3));
        CHECK((info.pv[1] == e5 || info.pv[1] == e6));
      }
      THEN("The other searcher info is filled correctly") {
        CHECK(info.depth == 2);
        CHECK(std::abs(info.score) <= 0.001);
      }
    }

    WHEN("We run a deeper search") {
      limits.depth_limit = 4;
      searcher.search(gs, limits, info, stop_signal);
      THEN("The resulting pv has the right length") {
        CHECK(info.pv.size() == 4);
      }
    }
  }

  GIVEN("The position after e4") {
    GameState gs;
    gs.make_move(gs.convert_move("e2e4"));
    BasicAlphaBetaSearcher searcher(std::make_unique<BasicEvaluator>());
    WHEN("We evaluate the position to depth 1") {
      Move e5 = gs.convert_move("e7e5");
      Move e6 = gs.convert_move("e7e6");
      SearchLimits limits;
      SearchInfo info;
      bool stop_signal = false;
      limits.depth_limit = 1;
      auto res = searcher.search(gs, limits, info, stop_signal);
      THEN("The best move is e5 or e6.") {
        CHECK((res.second == e5 || res.second == e6));
      }
      THEN("The best move has evaluation zero.") {
        CHECK(std::abs(res.first) <= 0.001);
      }
    }
  }

  GIVEN("A position with mate in 2 for black") {
    GameState gs("2K5/8/2k5/8/8/8/8/3q4 b - - 0 1");
    Move qd7 = gs.convert_move("d1d7");
    gs.make_move(qd7);
    Move kb8 = gs.convert_move("c8b8");
    gs.make_move(kb8);
    Move qb7 = gs.convert_move("d7b7");
    gs.undo_move();
    gs.undo_move();
    BasicAlphaBetaSearcher searcher(std::make_unique<BasicEvaluator>());
    WHEN("We evaluate withd epth 3") {
      SearchLimits limits;
      SearchInfo info;
      limits.mate_in = 2;
      bool stop_signal = false;
      auto res = searcher.search(gs, limits, info, stop_signal);
      THEN("The evaluation is largely negative") {
        CHECK(res.first < -100.0);
      }
      THEN("The principle variation is the mate") {
        CHECK(info.pv.size() == 3);
        CHECK(info.pv[0] == qd7);
        CHECK(info.pv[1] == kb8);
        CHECK(info.pv[2] == qb7);
      }
    }
  }
}
