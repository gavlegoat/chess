#pragma once

#include <vector>
#include <string>

/**
 * \brief Split a string according to a given delimiter.
 */
std::vector<std::string> split(const std::string& inp, char delim);

/**
 * \brief Convert an algebraic square name to an integer identifier.
 */
int algebraic_to_int(std::string algebraic);

/**
 * \brief Convert an integer square identifer to an algebraic name.
 */
std::string int_to_algebraic(int pos);
