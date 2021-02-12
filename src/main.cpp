#include <iostream>
#include <memory>
#include <vector>
#include <cctype>

#include "movegen.hpp"
#include "search.hpp"
#include "evaluation.hpp"

/**
 * \brief The main class holding a chess engine.
 *
 * This class is responsible for handling threads so that the main loop can
 * continue to interact with the GUI.
 */
class Engine {
  private:
    std::unique_ptr<Searcher> searcher;

  public:
    Engine(std::unique_ptr<Searcher>&& s): searcher{std::move(s)} {}
};

std::vector<std::string> split(const std::string& inp) {
  std::vector<std::string> ret;
  std::string cur = "";
  for (char c : inp) {
    if (isspace(c)) {
      if (!cur.empty()) {
        ret.push_back(cur);
        cur = "";
      }
    } else {
      cur.push_back(c);
    }
  }
  if (!cur.empty()) {
    ret.push_back(cur);
  }
  return ret;
}

int main(int argc, char** argv) {
  bool boards_initialized = false;

  bool debug = false;
  GameState gs;

  std::unique_ptr<Evaluator> eval = std::make_unique<BasicEvaluator>();
  std::unique_ptr<Searcher> search = std::make_unique<BasicAlphaBetaSearcher>(std::move(eval));
  Engine engine(std::move(search));

  // Handle UCI commands
  for (std::string line; std::getline(std::cin, line);) {
    std::vector<std::string> tokens = split(line);
    if (tokens[0] == "uci") {
      if (tokens.size() > 1) {
        throw std::runtime_error("Unexpected argument to command uci");
      }
      std::cout << "id name Test" << std::endl;
      std::cout << "id author Greg Anderson" << std::endl;
      // We should send option information here once it exists.
      std::cout << "uciok" << std::endl;
    } else if (tokens[0] == "debug") {
      if (tokens.size() != 2) {
        throw std::runtime_error("Wrong number of arguments to command debug");
      }
      if (tokens[1] == "on") {
        debug = true;
      } else if (tokens[1] == "off") {
        debug = false;
      } else {
        throw std::runtime_error("Unexpected argument to command debug");
      }
    } else if (tokens[0] == "isready") {
      movegen_initialize_attack_boards();
      boards_initialized = false;
      std::cout << "readyok" << std::endl;
    } else if (tokens[0] == "setoption") {
      // There are currently no options that can be set.
      throw std::runtime_error("Unrecognized option in setoption");
    } else if (tokens[0] == "register") {
      // There is no required registration
      throw std::runtime_error("No registration requested");
    } else if (tokens[0] == "ucinewgame") {
      // We currently don't have anythin advanced enough to care about this.
    } else if (tokens[0] == "position") {
      if (tokens.size() < 2) {
        throw std::runtime_error("Not enough arguments to command position");
      }
      unsigned index = 0;
      if (tokens[1] == "fen") {
        if (tokens.size() < 3) {
          throw std::runtime_error("Expected FEN string after position fen");
        }
        index = 2;
        std::string fen;
        while (index < tokens.size() && tokens[index] != "moves") {
          if (index > 2) {
            fen += " ";
          }
          fen += tokens[index];
          index++;
        }
        gs = GameState(fen);
      } else if (tokens[1] == "startpos") {
        gs = GameState();
        index = 2;
      } else {
        throw std::runtime_error("Unknown position type");
      }
      if (tokens.size() > index) {
        if (tokens[index] == "moves") {
          for (unsigned i = index + 1; i < tokens.size(); i++) {
            Move m = gs.convert_move(tokens[index]);
            gs.make_move(m);
          }
        } else {
          throw std::runtime_error("Unrecognized arguments to command position");
        }
      }
    } else if (tokens[0] == "go") {
      int ind = 1;
      while (ind < tokens.size()) {
        if (tokens[ind] == "searchmoves") {
          // TODO
        } else if (tokens[ind] == "ponder") {
          // TODO
        } else if (tokens[ind] == "wtime") {
          // White time left, currently unused
        } else if (tokens[ind] == "btime") {
          // Black time left, currently unused
        } else if (tokens[ind] == "winc") {
          // White increment, currently unused
        } else if (tokens[ind] == "binc") {
          // Black increment, currently unused
        } else if (tokens[ind] == "movestogo") {
          // Moves to the time control, currently unused
        } else if (tokens[ind] == "depth") {
          // TODO
        } else if (tokens[ind] == "nodes") {
          // TODO
        } else if (tokens[ind] == "mate") {
          // TODO
        } else if (tokens[ind] == "movetime") {
          // TODO
        } else if (tokens[ind] == "infinite") {
          // TODO
        }
        ind++;
      }
    } else if (tokens[0] == "stop") {
      // TODO
    } else if (tokens[0] == "ponderhit") {
      // TODO
    } else if (tokens[0] == "quit") {
      break;
    } else {
      throw std::runtime_error("Unrecognzized command");
    }
  }

  if (boards_initialized) {
    movegen_free_magics();
  }
}
