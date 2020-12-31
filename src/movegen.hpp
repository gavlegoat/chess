#pragma once

#include <deque>

#include "boards.hpp"

// For now a move list is just a queue, but this may change later in order
// to support better move ordering.
typedef std::deque<Move> MoveList;

/**
 * \brief Generate a list of legal moves.
 */
MoveList generate_moves(const GameState& gs);

/**
 * \brief Determine whether the player who's turn it is is in check.
 */
bool in_check(bool white_to_move, const Position& p);

/**
 * \brief Precompute some data to speed up move generation.
 *
 * This function generates bitboards which can be used to quickly generate
 * moves for certain kinds of pieces. This includes attack boards for the
 * knight and king, along with magic bitboards for the rooks and bishops.
 * See movegen.cpp for a brief explanation of magic bitboards. This function
 * should be called once when the engine starts up.
 */
void movegen_initialize_attack_boards();

/**
 * \brief Free the precomputed data.
 *
 * This function should be called once when the engine shuts down.
 */
void movegen_free_magics();

