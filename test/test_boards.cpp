#include "catch.hpp"

#include "boards.hpp"

SCENARIO("pieces can be placed on and removed from positions") {
  GIVEN("a board with some pieces on it") {
    Position p();
    p.place_piece(0, Position::W_ROOK);
    p.place_piece(1, Position::W_KNIGHT);
    p.place_piece(2, Position::W_BISHOP);
    p.place_piece(3, Position::W_QUEEN);
    p.place_piece(4, Position::W_KING);
    p.place_piece(8, Position::W_PAWN);
    p.place_piece(63, Position::B_ROOK);
    p.place_piece(62, Position::B_KNIGHT);
    p.place_piece(61, Position::B_BISHOP);
    p.place_piece(60, Position::B_KING);
    p.place_piece(59, Position::B_QUEEN);
    p.place_piece(55, Position::B_PAWN);

    CHECK(p.fen_board() == "3qkbnr/7p/8/8/8/8/P7/RNBQK3");

    WHEN("we add pieces") {
      p.place_piece(7, Position::W_ROOK);
      p.place_piece(56, Position::B_ROOK);
      p.place_piece(28, Position::W_PAWN);

      THEN("those pieces are on the board") {
        CHECK(p.fen_board() == "r2qkbnr/7p/8/8/4p3/8/P7/RNBQK2R");
      }
    }

    WHEN("we remove pieces") {
      p.remove_piece(0, Position::W_ROOK);
      p.remove_piece(8, Position::W_PAWN);

      THEN("those pieces are removed") {
        CHECK(p.fen_baord() == "3qkbnr/7p/8/8/8/8/8/1NBQK3");
      }
    }

    WHEN("we remove pieces that aren't there") {
      p.remove_piece(28, Position::W_PAWN);
      p.remove_piece(0, Position::W_BISHOP);

      THEN("nothing happens") {
        CHECK(p.fen_board() == "3qkbnr/7p/8/8/8/8/P7/RNBQK3");
      }
    }
  }
}

TEST_CASE("a position can be tested for piece occupation") {
  Position p();
  p.place_piece(0, Position::W_ROOK);
  p.place_piece(1, Position::W_KNIGHT);
  p.place_piece(2, Position::W_BISHOP);
  p.place_piece(3, Position::W_QUEEN);
  p.place_piece(4, Position::W_KING);
  p.place_piece(8, Position::W_PAWN);
  p.place_piece(63, Position::B_ROOK);
  p.place_piece(62, Position::B_KNIGHT);
  p.place_piece(61, Position::B_BISHOP);
  p.place_piece(60, Position::B_KING);
  p.place_piece(59, Position::B_QUEEN);
  p.place_piece(55, Position::B_PAWN);

  CHECK(p.piece_at(0, Position::W_ROOK));
  CHECK(p.piece_at(55, Position::B_PAWN));
  CHECK(!p.piece_at(28, Position::W_PAWN));
  CHECK(!p.piece_at(0, Position::W_PAWN));
}

SCENARIO("moves can be made on positions") {
  // TODO
}

SCENARIO("we can make moves on a game state") {
  // TODO
}

TEST_CASE("a fen string can be generated from a game state") {
  // TODO
}
