#ifndef _MOVEGEN_H_
#define _MOVEGEN_H_

#include <deque>

#include "boards.hpp"

// For now a move list is just a queue, but this may change later in order
// to support better move ordering.
typedef std::deque<Move> MoveList;

// Generate a list of pseudo-legal moves for the given game state.
MoveList generate_moves(const GameState& gs);

#endif
