#include "catch.hpp"

#include "boards.hpp"

SCENARIO("pieces can be placed on and removed from positions") {
  GIVEN("a board with some pieces on it") {
    Position p;
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
        CHECK(p.fen_board() == "r2qkbnr/7p/8/8/4P3/8/P7/RNBQK2R");
      }
    }

    WHEN("we remove pieces") {
      p.remove_piece(0, Position::W_ROOK);
      p.remove_piece(8, Position::W_PAWN);

      THEN("those pieces are removed") {
        CHECK(p.fen_board() == "3qkbnr/7p/8/8/8/8/8/1NBQK3");
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
  Position p;
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
  GIVEN("a board in some starting position") {
    // Initial position:
    // . . . . r n b q
    // P P P P P P P .
    // . . . . . . . .
    // . . . . . . P p
    // r . . Q b . . .
    // . . . R . . . .
    // . . . . . . . .
    // R . . . K . . R
    Position p("4rnbq/PPPPPPP1/8/6Pp/r2Qb3/3R4/8/R3K2R");

    CHECK(p.fen_board() == "4rnbq/PPPPPPP1/8/6Pp/r2Qb3/3R4/8/R3K2R");

    WHEN("we make some moves") {
      Move m1 = Move(27, 26, Position::W_QUEEN, Move::QUIET);
      Move m2 = Move(39, 31, Position::B_PAWN, Move::QUIET);
      p.make_move(m1);
      p.make_move(m2);

      THEN("the resulting position is correct") {
        CHECK(p.fen_board() == "4rnbq/PPPPPPP1/8/6P1/r1Q1b2p/3R4/8/R3K2R");
      }
    }

    WHEN("we capture a piece") {
      Move m1 = Move(27, 24, Position::W_QUEEN, Move::CAPTURE);
      Move m2 = Move(28, 19, Position::B_BISHOP, Move::CAPTURE);
      p.make_move(m1);
      p.make_move(m2);

      THEN("the piece is removed") {
        CHECK(p.fen_board() == "4rnbq/PPPPPPP1/8/6Pp/Q7/3b4/8/R3K2R");
      }
    }

    WHEN("we castle queenside") {
      Move m1 = Move(4, 2, Position::W_KING, Move::QUEEN_CASTLE);
      p.make_move(m1);

      THEN("the rook moves") {
        CHECK(p.fen_board() == "4rnbq/PPPPPPP1/8/6Pp/r2Qb3/3R4/8/2KR3R");
      }
    }

    WHEN("we castle kingside") {
      Move m1 = Move(4, 6, Position::W_KING, Move::KING_CASTLE);
      p.make_move(m1);

      THEN("the rook moves") {
        CHECK(p.fen_board() == "4rnbq/PPPPPPP1/8/6Pp/r2Qb3/3R4/8/R4RK1");
      }
    }

    WHEN("we capture en passant") {
      Move m1 = Move(38, 47, Position::W_PAWN, Move::CAPTURE_EP);
      p.make_move(m1);

      THEN("the captured pawn is removed") {
        CHECK(p.fen_board() == "4rnbq/PPPPPPP1/7P/8/r2Qb3/3R4/8/R3K2R");
      }
    }

    WHEN("we promote a pawn") {
      Move m1 = Move(48, 56, Position::W_PAWN, Move::PROMOTE_KNIGHT);
      Move m2 = Move(49, 57, Position::W_PAWN, Move::PROMOTE_BISHOP);
      Move m3 = Move(50, 58, Position::W_PAWN, Move::PROMOTE_ROOK);
      Move m4 = Move(51, 59, Position::W_PAWN, Move::PROMOTE_QUEEN);
      p.make_move(m1);
      p.make_move(m2);
      p.make_move(m3);
      p.make_move(m4);

      THEN("the given piece is added to the board") {
        CHECK(p.fen_board() == "NBRQrnbq/4PPP1/8/6Pp/r2Qb3/3R4/8/R3K2R");
      }
    }

    WHEN("we promote with capture") {
      Move m1 = Move(51, 60, Position::W_PAWN, Move::PROMOTE_KNIGHT_CAPTURE);
      Move m2 = Move(52, 61, Position::W_PAWN, Move::PROMOTE_BISHOP_CAPTURE);
      Move m3 = Move(53, 62, Position::W_PAWN, Move::PROMOTE_ROOK_CAPTURE);
      Move m4 = Move(54, 63, Position::W_PAWN, Move::PROMOTE_QUEEN_CAPTURE);
      p.make_move(m1);
      p.make_move(m2);
      p.make_move(m3);
      p.make_move(m4);

      THEN("the given piece is added to the board") {
        CHECK(p.fen_board() == "4NBRQ/PPP5/8/6Pp/r2Qb3/3R4/8/R3K2R");
      }
    }
  }
}

TEST_CASE("a fen string can be generated from a game state") {
  GameState gs;
  CHECK(gs.fen_string() == "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

  Move m1(12, 28, Position::W_PAWN, Move::PAWN_DOUBLE);
  gs.make_move(m1);
  CHECK(gs.fen_string() == "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");

  Move m2(50, 34, Position::B_PAWN, Move::PAWN_DOUBLE);
  gs.make_move(m2);
  CHECK(gs.fen_string() == "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2");

  Move m3(6, 21, Position::W_KNIGHT, Move::QUIET);
  gs.make_move(m3);
  CHECK(gs.fen_string() == "rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2");
}

SCENARIO("we can make moves on a game state") {
  // Since we already know that moves can be made on a position, here we only
  // check for game state specific problems, like castling and checking for en
  // passant possiblities.

  GIVEN("a game state") {
    Position p("r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R");
    GameState gs(p, true, true, true, true, true, 0, false, 0, 1,
        std::map<Position, int>(), std::deque<Node>());

    CHECK(gs.fen_string() == "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1");

    WHEN("we castle kingside") {
      Move m1(4, 2, Position::W_KING, Move::QUEEN_CASTLE);
      Move m2(60, 58, Position::B_KING, Move::QUEEN_CASTLE);
      gs.make_move(m1);
      gs.make_move(m2);

      THEN("castling rights are removed") {
        CHECK(gs.fen_string() == "2kr3r/pppppppp/8/8/8/8/PPPPPPPP/2KR3R w - - 2 2");
      }
    }

    WHEN("we castle queenside") {
      Move m1(4, 6, Position::W_KING, Move::KING_CASTLE);
      Move m2(60, 62, Position::B_KING, Move::KING_CASTLE);
      gs.make_move(m1);
      gs.make_move(m2);

      THEN("castling rights are removed") {
        CHECK(gs.fen_string() == "r4rk1/pppppppp/8/8/8/8/PPPPPPPP/R4RK1 w - - 2 2");
      }
    }

    WHEN("we move a rook") {
      Move m1(0, 1, Position::W_ROOK, Move::QUIET);
      Move m2(63, 62, Position::B_ROOK, Move::QUIET);
      gs.make_move(m1);
      gs.make_move(m2);

      THEN("castling rights are removed") {
        CHECK(gs.fen_string() == "r3k1r1/pppppppp/8/8/8/8/PPPPPPPP/1R2K2R w Kq - 2 2");
      }
    }

    WHEN("en passant becomes possible") {
      Move m1(8, 24, Position::W_PAWN, Move::PAWN_DOUBLE);
      Move m2(55, 39, Position::B_PAWN, Move::PAWN_DOUBLE);
      Move m3(24, 32, Position::W_PAWN, Move::QUIET);
      Move m4(39, 31, Position::B_PAWN, Move::QUIET);
      Move m5(14, 30, Position::W_PAWN, Move::PAWN_DOUBLE);
      gs.make_move(m1);
      gs.make_move(m2);
      gs.make_move(m3);
      gs.make_move(m4);
      gs.make_move(m5);

      THEN("the game state permits en passant") {
        CHECK(gs.fen_string() == "r3k2r/ppppppp1/8/P7/6Pp/8/1PPPPP1P/R3K2R b KQkq g3 0 3");
      }
    }

    WHEN("the en passant capture isn't taken immediately") {
      Move m1(8, 24, Position::W_PAWN, Move::PAWN_DOUBLE);
      Move m2(55, 39, Position::B_PAWN, Move::PAWN_DOUBLE);
      Move m3(24, 32, Position::W_PAWN, Move::QUIET);
      Move m4(49, 33, Position::B_PAWN, Move::PAWN_DOUBLE);
      Move m5(0, 1, Position::W_ROOK, Move::QUIET);
      Move m6(39, 31, Position::B_PAWN, Move::QUIET);
      gs.make_move(m1);
      gs.make_move(m2);
      gs.make_move(m3);
      gs.make_move(m4);
      gs.make_move(m5);
      gs.make_move(m6);

      THEN("the en passant opportunity goes away") {
        CHECK(gs.fen_string() == "r3k2r/p1ppppp1/8/Pp6/7p/8/1PPPPPPP/1R2K2R w Kkq - 0 4");
      }
    }
  }
}
