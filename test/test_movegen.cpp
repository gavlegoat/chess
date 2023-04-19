#include "catch.hpp"

#include <algorithm>

#include "movegen.hpp"
#include "boards.hpp"
#include "utils.hpp"

// Perft testing is slow so we allow it to be disabled.
// Better to specify this in cmake if you want to use perft testing.
//#define ENABLE_PERFT

void debug_move_list(const MoveList& l) {
  for (const Move& m : l) {
    std::cout << m.piece() << " " << int_to_algebraic(m.from_square()) << "-" << int_to_algebraic(m.to_square()) << " (" << m.get_flags() << ")" << std::endl;
  }
}

uint64_t perft(int depth, GameState start) {
  if (depth == 0) {
    return 1;
  }
  uint64_t n = 0;
  MoveList moves = generate_moves(start);
  //debug_move_list(moves);
  for (const Move& m : moves) {
    start.make_move(m);
    uint64_t p = perft(depth - 1, start);
    //if (depth == 2) {
    //  std::cout << m.piece() << " " << int_to_algebraic(m.from_square()) << "-" << int_to_algebraic(m.to_square()) << " (" << m.get_flags() << ")" << std::endl;
    //  std::cout << p << std::endl;
    //}
    n += p;
    start.undo_move();
  }

  return n;
}

bool contains_move(const MoveList& l, const Move& m) {
  return std::find(l.begin(), l.end(), m) != l.end();
}

SCENARIO("king moves are generated correctly") {

  GIVEN("a board with just a king") {
    GameState gs("8/8/8/4K3/8/8/8/3k4 w - - 0 1");
    MoveList l = generate_moves(gs);
    Move kd4(algebraic_to_int("e5"), algebraic_to_int("d4"), Position::W_KING, Move::QUIET);
    Move kd5(algebraic_to_int("e5"), algebraic_to_int("d5"), Position::W_KING, Move::QUIET);
    Move kd6(algebraic_to_int("e5"), algebraic_to_int("d6"), Position::W_KING, Move::QUIET);
    Move ke4(algebraic_to_int("e5"), algebraic_to_int("e4"), Position::W_KING, Move::QUIET);
    Move ke6(algebraic_to_int("e5"), algebraic_to_int("e6"), Position::W_KING, Move::QUIET);
    Move kf4(algebraic_to_int("e5"), algebraic_to_int("f4"), Position::W_KING, Move::QUIET);
    Move kf5(algebraic_to_int("e5"), algebraic_to_int("f5"), Position::W_KING, Move::QUIET);
    Move kf6(algebraic_to_int("e5"), algebraic_to_int("f6"), Position::W_KING, Move::QUIET);

    CHECK(l.size() == 8);
    CHECK(contains_move(l, kd4));
    CHECK(contains_move(l, kd5));
    CHECK(contains_move(l, kd6));
    CHECK(contains_move(l, ke4));
    CHECK(contains_move(l, ke6));
    CHECK(contains_move(l, kf4));
    CHECK(contains_move(l, kf5));
    CHECK(contains_move(l, kf6));
  }

  GIVEN("a king with some squares leading to check") {
    GameState gs("8/3N4/2K1B3/8/4k3/8/8/8 b - - 0 1");
    // . . . . . . . .
    // . . . N . . . .   c = check
    // . . K . B . . .   x = legal move
    // . . . c c c . .
    // . . . x k x . .
    // . . . x x x . .
    // . . . . . . . .
    // . . . . . . . .
    MoveList l = generate_moves(gs);
    Move kd4(algebraic_to_int("e4"), algebraic_to_int("d4"), Position::B_KING, Move::QUIET);
    Move kf4(algebraic_to_int("e4"), algebraic_to_int("f4"), Position::B_KING, Move::QUIET);
    Move kd3(algebraic_to_int("e4"), algebraic_to_int("d3"), Position::B_KING, Move::QUIET);
    Move ke3(algebraic_to_int("e4"), algebraic_to_int("e3"), Position::B_KING, Move::QUIET);
    Move kf3(algebraic_to_int("e4"), algebraic_to_int("f3"), Position::B_KING, Move::QUIET);

    CHECK(l.size() == 5);
    CHECK(contains_move(l, kd4));
    CHECK(contains_move(l, kf4));
    CHECK(contains_move(l, kd3));
    CHECK(contains_move(l, ke3));
    CHECK(contains_move(l, kf3));
  }

  GIVEN("castling opportunities") {
    GameState gs("k7/p7/8/8/8/8/8/R3K2R w KQkq - 0 1");
    MoveList l = generate_moves(gs);
    Move oo(algebraic_to_int("e1"), algebraic_to_int("g1"), Position::W_KING, Move::KING_CASTLE);
    Move ooo(algebraic_to_int("e1"), algebraic_to_int("c1"), Position::W_KING, Move::QUEEN_CASTLE);

    CHECK(l.size() == 25);
    CHECK(contains_move(l, oo));
    CHECK(contains_move(l, ooo));
  }

}

SCENARIO("pawn moves are generated correctly") {

  GIVEN("a board with just pawns") {
    GameState gs("k7/4p3/3p4/8/8/8/1p6/7K b - - 0 1");
    MoveList l = generate_moves(gs);
    Move e6(algebraic_to_int("e7"), algebraic_to_int("e6"), Position::B_PAWN, Move::QUIET);
    Move e5(algebraic_to_int("e7"), algebraic_to_int("e5"), Position::B_PAWN, Move::PAWN_DOUBLE);
    Move d5(algebraic_to_int("d6"), algebraic_to_int("d5"), Position::B_PAWN, Move::QUIET);
    Move b1Q(algebraic_to_int("b2"), algebraic_to_int("b1"), Position::B_PAWN, Move::PROMOTE_QUEEN);
    Move b1R(algebraic_to_int("b2"), algebraic_to_int("b1"), Position::B_PAWN, Move::PROMOTE_ROOK);
    Move b1B(algebraic_to_int("b2"), algebraic_to_int("b1"), Position::B_PAWN, Move::PROMOTE_BISHOP);
    Move b1N(algebraic_to_int("b2"), algebraic_to_int("b1"), Position::B_PAWN, Move::PROMOTE_KNIGHT);

    CHECK(l.size() == 10);
    CHECK(contains_move(l, e6));
    CHECK(contains_move(l, e5));
    CHECK(contains_move(l, d5));
    CHECK(contains_move(l, b1Q));
    CHECK(contains_move(l, b1R));
    CHECK(contains_move(l, b1B));
    CHECK(contains_move(l, b1N));
  }

  GIVEN("capturing options including en passant") {
    GameState gs("k7/8/2p5/3Pp3/8/8/8/7K w - e6 0 1");
    MoveList l = generate_moves(gs);
    Move c6(algebraic_to_int("d5"), algebraic_to_int("c6"), Position::W_PAWN, Move::CAPTURE);
    Move d6(algebraic_to_int("d5"), algebraic_to_int("d6"), Position::W_PAWN, Move::QUIET);
    Move e6(algebraic_to_int("d5"), algebraic_to_int("e6"), Position::W_PAWN, Move::CAPTURE_EP);

    CHECK(l.size() == 6);
    CHECK(contains_move(l, c6));
    CHECK(contains_move(l, d6));
    CHECK(contains_move(l, e6));
  }

}

SCENARIO("rook moves are generated correctly") {

  GIVEN("a board with just rooks") {
    GameState gs("k7/8/8/8/8/3R4/8/7K w - - 0 1");
    MoveList l = generate_moves(gs);
    Move rd2(algebraic_to_int("d3"), algebraic_to_int("d2"), Position::W_ROOK, Move::QUIET);
    Move rf3(algebraic_to_int("d3"), algebraic_to_int("f3"), Position::W_ROOK, Move::QUIET);
    Move rd8(algebraic_to_int("d3"), algebraic_to_int("d8"), Position::W_ROOK, Move::QUIET);
    Move ra3(algebraic_to_int("d3"), algebraic_to_int("a3"), Position::W_ROOK, Move::QUIET);

    CHECK(l.size() == 17);
    CHECK(contains_move(l, rd2));
    CHECK(contains_move(l, rf3));
    CHECK(contains_move(l, rd8));
    CHECK(contains_move(l, ra3));
  }

  GIVEN("a board with rooks that are blocked") {
    GameState gs("k7/8/2p5/8/2r3P1/8/8/7K b - - 0 1");
    // k . . . . . . .
    // . . . . . . . .
    // . . p . . . . .
    // . . x . . . . .
    // x x r x x x P .
    // . . x . . . . .
    // . . x . . . . .
    // . . x . . . . K
    MoveList l = generate_moves(gs);
    Move rg4(algebraic_to_int("c4"), algebraic_to_int("g4"), Position::B_ROOK, Move::CAPTURE);
    Move rc8(algebraic_to_int("c4"), algebraic_to_int("c8"), Position::B_ROOK, Move::QUIET);
    Move rc6(algebraic_to_int("c4"), algebraic_to_int("c6"), Position::B_ROOK, Move::CAPTURE);
    Move rh4(algebraic_to_int("c4"), algebraic_to_int("c8"), Position::B_ROOK, Move::QUIET);

    CHECK(l.size() == 14);
    CHECK(contains_move(l, rg4));
    CHECK(!contains_move(l, rc8));
    CHECK(!contains_move(l, rc6));
    CHECK(!contains_move(l, rh4));
  }

}

SCENARIO("bishop moves are generated correctly") {

  GIVEN("a board with just bishops") {
    GameState gs("k7/8/8/5b2/8/8/8/7K b - - 0 1");
    // k . x . . . . .
    // . . . x . . . x
    // . . . . x . x .
    // . . . . . b . .
    // . . . . x . x .
    // . . . x . . . x
    // . . x . . . . .
    // . x . . . . . K
    MoveList l = generate_moves(gs);
    Move bh7(algebraic_to_int("f5"), algebraic_to_int("h7"), Position::B_BISHOP, Move::QUIET);
    Move bd3(algebraic_to_int("f5"), algebraic_to_int("d3"), Position::B_BISHOP, Move::QUIET);
    Move bc8(algebraic_to_int("f5"), algebraic_to_int("c8"), Position::B_BISHOP, Move::QUIET);
    Move bg4(algebraic_to_int("f5"), algebraic_to_int("g4"), Position::B_BISHOP, Move::QUIET);

    CHECK(l.size() == 14);
    CHECK(contains_move(l, bh7));
    CHECK(contains_move(l, bd3));
    CHECK(contains_move(l, bc8));
    CHECK(contains_move(l, bg4));
  }

}

SCENARIO("queen moves are generated correctly") {

  GIVEN("a board with just queens") {
    GameState gs("k7/8/5Q2/8/8/8/8/7K w - - 0 1");
    // k . . x . x . x
    // . . . . x x x .
    // x x x x x Q x x
    // . . . . x x x .
    // . . . x . x . x
    // . . x . . x . .
    // . x . . . x . .
    // x . . . . x . K
    MoveList l = generate_moves(gs);
    Move qb6(algebraic_to_int("f6"), algebraic_to_int("b6"), Position::W_QUEEN, Move::QUIET);
    Move qh4(algebraic_to_int("f6"), algebraic_to_int("h4"), Position::W_QUEEN, Move::QUIET);

    CHECK(l.size() == 28);
    CHECK(contains_move(l, qb6));
    CHECK(contains_move(l, qh4));
  }

}

SCENARIO("knight moves are generated correctly") {

  GIVEN("a board with only knights") {
    GameState gs("k7/8/8/4n3/8/8/8/7K b - - 0 1");
    MoveList l = generate_moves(gs);
    Move nf7(algebraic_to_int("e5"), algebraic_to_int("f7"), Position::B_KNIGHT, Move::QUIET);
    Move ng6(algebraic_to_int("e5"), algebraic_to_int("g6"), Position::B_KNIGHT, Move::QUIET);
    Move ng4(algebraic_to_int("e5"), algebraic_to_int("g4"), Position::B_KNIGHT, Move::QUIET);
    Move nf3(algebraic_to_int("e5"), algebraic_to_int("f3"), Position::B_KNIGHT, Move::QUIET);
    Move nd3(algebraic_to_int("e5"), algebraic_to_int("d3"), Position::B_KNIGHT, Move::QUIET);
    Move nc4(algebraic_to_int("e5"), algebraic_to_int("c4"), Position::B_KNIGHT, Move::QUIET);
    Move nc6(algebraic_to_int("e5"), algebraic_to_int("c6"), Position::B_KNIGHT, Move::QUIET);
    Move nd7(algebraic_to_int("e5"), algebraic_to_int("d7"), Position::B_KNIGHT, Move::QUIET);

    CHECK(l.size() == 11);
    CHECK(contains_move(l, nf7));
    CHECK(contains_move(l, ng6));
    CHECK(contains_move(l, ng4));
    CHECK(contains_move(l, nf3));
    CHECK(contains_move(l, nd3));
    CHECK(contains_move(l, nc4));
    CHECK(contains_move(l, nc6));
    CHECK(contains_move(l, nd7));
  }

  GIVEN("a board where a knight can capture") {
    GameState gs("k7/8/8/4n3/8/5P2/8/7K b - - 0 1");
    MoveList l = generate_moves(gs);
    Move nf7(algebraic_to_int("e5"), algebraic_to_int("f7"), Position::B_KNIGHT, Move::QUIET);
    Move ng6(algebraic_to_int("e5"), algebraic_to_int("g6"), Position::B_KNIGHT, Move::QUIET);
    Move ng4(algebraic_to_int("e5"), algebraic_to_int("g4"), Position::B_KNIGHT, Move::QUIET);
    Move nf3(algebraic_to_int("e5"), algebraic_to_int("f3"), Position::B_KNIGHT, Move::CAPTURE);
    Move nd3(algebraic_to_int("e5"), algebraic_to_int("d3"), Position::B_KNIGHT, Move::QUIET);
    Move nc4(algebraic_to_int("e5"), algebraic_to_int("c4"), Position::B_KNIGHT, Move::QUIET);
    Move nc6(algebraic_to_int("e5"), algebraic_to_int("c6"), Position::B_KNIGHT, Move::QUIET);
    Move nd7(algebraic_to_int("e5"), algebraic_to_int("d7"), Position::B_KNIGHT, Move::QUIET);

    CHECK(l.size() == 11);
    CHECK(contains_move(l, nf7));
    CHECK(contains_move(l, ng6));
    CHECK(contains_move(l, ng4));
    CHECK(contains_move(l, nf3));
    CHECK(contains_move(l, nd3));
    CHECK(contains_move(l, nc4));
    CHECK(contains_move(l, nc6));
    CHECK(contains_move(l, nd7));
  }

}

TEST_CASE("We can tell when the king is in check") {
  Position p("k7/6r1/8/8/8/8/6K1/8");
  CHECK(in_check(true, p));
  CHECK(!in_check(false, p));
}

#ifdef ENABLE_PERFT

SCENARIO("perft testing gives correct results") {

  GIVEN("the starting position") {
    GameState gs("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    CHECK(perft(1, gs) == 20);
    CHECK(perft(2, gs) == 400);
    CHECK(perft(3, gs) == 8902);
    CHECK(perft(4, gs) == 197281);
  }

  GIVEN("a 2nd position") {
    GameState gs("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");

    CHECK(perft(1, gs) == 48);
    CHECK(perft(2, gs) == 2039);
    CHECK(perft(3, gs) == 97862);
  }

  GIVEN("a 3rd position") {
    GameState gs("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");

    CHECK(perft(1, gs) == 14);
    CHECK(perft(2, gs) == 191);
    CHECK(perft(3, gs) == 2812);
    CHECK(perft(4, gs) == 43238);
  }

  GIVEN("a 4th position") {
    GameState gs("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");

    CHECK(perft(1, gs) == 44);
    CHECK(perft(2, gs) == 1486);
    CHECK(perft(3, gs) == 62379);
    CHECK(perft(4, gs) == 2103487);
  }

}
#endif
