#ifndef _MOVEGEN_H_
#define _MOVEGEN_H_

#include <deque>

#include "boards.hpp"

// For now a move list is just a queue, but this may change later in order
// to support better move ordering.
typedef std::deque<Move> MoveList;

// Generate a list of legal moves for the given game state.
MoveList generate_moves(const GameState& gs);

// Initialize a set of bitboards which will be useful for generating moves.
// This function should be called once on program startup.
void movegen_initialize_attack_boards();

// Free the magic bitboard arrays.
void movegen_free_magics();

#endif
