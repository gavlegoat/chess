#pragma once

#include <vector>
#include <string>

std::vector<std::string> split(const std::string& inp, char delim);

// Convert an algebraic position to an index
int algebraic_to_int(std::string algebraic);

// Convert an index to algebraic notation
std::string int_to_algebraic(int pos);
