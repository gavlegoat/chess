#include <sstream>

#include "utils.hpp"

std::vector<std::string> split(const std::string& inp, char delim) {
  std::istringstream iss(inp);
  std::vector<std::string> tokens;
  std::string token;
  while (std::getline(iss, token, delim)) {
    tokens.push_back(token);
  }
  return tokens;
}

int algebraic_to_int(std::string algebraic) {
  int file = algebraic[0] - 'a';
  int rank = algebraic[1] - '1';
  if (file < 0 || file >= 8) {
    throw std::domain_error("Cannot convert from algebraic notation: file is not between 'a' and 'h'");
  }
  if (rank < 0 || rank >= 8) {
    throw std::domain_error("Cannot convert from algebraic notation: rank is not between 1 and 8");
  }
  return rank * 8 + file;
}

std::string int_to_algebraic(int pos) {
  if (pos < 0 || pos >= 64) {
    throw std::domain_error("Cannot convert to algebraic notation: pos should be between 0 and 63");
  }
  int rank = 1 + pos / 8;
  char file = 'a' + pos % 8;
  return std::string(1, file) + std::to_string(rank);
}
