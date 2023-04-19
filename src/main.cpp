#include <iostream>
#include <memory>
#include <vector>
#include <cctype>
#include <thread>
#include <optional>
#include <chrono>
#include <functional>

#include "movegen.hpp"
#include "search.hpp"
#include "evaluation.hpp"

#define DEFAULT_WRITE_PERIOD 500

using namespace std::chrono_literals;

/**
 * \brief Track resource limits and provide status updates for a search.
 *
 * This function is responsible for keeping track of time limits in a search
 * and for providing search info to the interface.
 *
 * \param limits The resource limits on the current search.
 * \param info Information about the curren search.
 * \param stop_signal Indicates that the search is out of resources.
 * \param write_period The frequence to write data to the interface.
 */
void report(const SearchLimits& limits, SearchInfo& info,
    bool& stop_signal, unsigned write_period) {
  auto start = std::chrono::system_clock::now();
  auto last_write = start;
  while (true) {
    // Wait a short time between iterations
    std::this_thread::sleep_for(10ms);

    // Update info
    auto current = std::chrono::system_clock::now();
    unsigned elapsed = (current - start) / 1ms;
    info.time = elapsed;

    // Stop if the timeout has passed
    bool stop = limits.timeout && elapsed > *limits.timeout;
    // We use ||= here in case another thread has signaled a stop
    stop_signal = stop_signal || stop;

    // Limit writes to every `write_period` ms in order to avoid overwhelming
    // the GUI
    if ((current - last_write) / 1ms >= write_period) {
      // Write out info
      std::cout << "info score cp " << (int) (info.score * 100) << " depth " <<
        info.depth << " nodes " << info.nodes << " time " << info.time <<
        " pv ";
      info.pv_lock.lock();
      for (const Move& m : info.pv) {
        std::cout << m << " ";
      }
      info.pv_lock.unlock();
      std::cout << std::endl;
      last_write = current;
    }

    if (stop_signal) {
      return;
    }
  }
}

/**
 * \brief A wrapper to help the searcher work with threads.
 */
void search_helper(Searcher& searcher, GameState& gs, const SearchLimits& limits,
    SearchInfo& info, bool& stop_signal) {
  searcher.search(gs, limits, info, stop_signal);
}

/**
 * \brief The main class holding a chess engine.
 *
 * This class is responsible for handling threads so that the main loop can
 * continue to interact with the GUI.
 */
class Engine {
  private:
    /** The search algorithm to use for this engine. */
    std::unique_ptr<Searcher> searcher;
    /** The thread responsible for the game search. */
    std::optional<std::thread> work_thread;
    /** The thread responsible for monitoring time limits. */
    std::optional<std::thread> timer_thread;
    /** A signal variable for telling the work/timer threads to halt. */
    bool stop_signal;
    /** The best available move, to be set by the work thread. */
    std::optional<Move> best_move;
    SearchInfo info;

  public:
    Engine(std::unique_ptr<Searcher>&& s): searcher{std::move(s)},
      work_thread{}, timer_thread{}, stop_signal{false}, best_move{},
      info{} {}

    /**
     * \brief Start searching with the speficied limits.
     *
     * \param limits Limitations which can be placed on search time.
     */
    void start(SearchLimits limits, GameState& gs) {
      searcher->initialize(gs);
      timer_thread = std::thread(report, std::cref(limits), std::ref(info),
          std::ref(stop_signal), DEFAULT_WRITE_PERIOD);
      work_thread = std::thread(search_helper, std::ref(*searcher),
          std::ref(gs), std::cref(limits), std::ref(info),
          std::ref(stop_signal));
    }

    /**
     * \brief Stop the current search and return the best move.
     *
     * \return The best move found by the searcher.
     */
    Move stop() {
      if (work_thread) {
        // NOTE: The two threads should always be started together
        stop_signal = true;
        work_thread->join();
        timer_thread->join();
        stop_signal = false;
      }
      work_thread = {};
      timer_thread = {};

      if (info.pv.empty()) {
        throw std::runtime_error("Stopped before finding any moves!");
      }
      return info.pv.front();
    }
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

bool is_move(const std::string& move) {
  return (move.length() == 4 || move.length() == 5) && 'a' <= move[0] &&
    move[0] <= 'h' && '1' <= move[1] && move[1] <= '8' && 'a' <= move[2] &&
    move[2] <= 'h' && '1' <= move[3] && move[3] <= '8';
}

int main(int argc, char** argv) {
  bool boards_initialized = false;

  GameState gs;
  std::optional<Move> ponder_move;

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
        // We don't do any debugging right now
      } else if (tokens[1] == "off") {
        // We don't do any debugging right now
      } else {
        throw std::runtime_error("Unexpected argument to command debug");
      }
    } else if (tokens[0] == "isready") {
      movegen_initialize_attack_boards();
      boards_initialized = true;
      std::cout << "readyok" << std::endl;
    } else if (tokens[0] == "setoption") {
      // There are currently no options that can be set.
      throw std::runtime_error("Unrecognized option in setoption");
    } else if (tokens[0] == "register") {
      // There is no required registration
    } else if (tokens[0] == "ucinewgame") {
      // We currently don't have anything advanced enough to care about this.
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
      } else if (tokens[1] == "moves") {
        index = 1;
      } else {
        throw std::runtime_error("Unrecognized argument in position");
      }
      if (tokens.size() > index) {
        if (tokens[index] == "moves") {
          for (unsigned i = index + 1; i < tokens.size(); i++) {
            Move m = gs.convert_move(tokens[i]);
            gs.make_move(m);
          }
        } else {
          throw std::runtime_error("Unrecognized arguments to command position");
        }
      }
    } else if (tokens[0] == "go") {
      SearchLimits limits;
      unsigned ind = 1;
      while (ind < tokens.size()) {
        if (tokens[ind] == "searchmoves") {
          ind++;
          limits.moves = MoveList();
          while (is_move(tokens[ind])) {
            Move m = gs.convert_move(tokens[ind]);
            ponder_move = m;
            limits.moves->push_back(m);
            ind++;
          }
          ind--;
        } else if (tokens[ind] == "ponder") {
          if (ponder_move) {
            gs.undo_move();
            limits.moves = {*ponder_move};
          }
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
          ind++;
          limits.depth_limit = std::stoi(tokens[ind]);
        } else if (tokens[ind] == "nodes") {
          ind++;
          limits.node_limit = std::stoi(tokens[ind]);
        } else if (tokens[ind] == "mate") {
          ind++;
          limits.mate_in = std::stoi(tokens[ind]);
        } else if (tokens[ind] == "movetime") {
          ind++;
          limits.timeout = std::stoi(tokens[ind]);
        } else if (tokens[ind] == "infinite") {
          // This is the default -- nothing to do
        }
        ind++;
      }
      engine.start(limits, gs);
    } else if (tokens[0] == "stop") {
      Move best = engine.stop();
      std::cout << "bestmove " << best << std::endl;
    } else if (tokens[0] == "ponderhit") {
      engine.stop();
      gs.make_move(*ponder_move);
      ponder_move = {};
      SearchLimits limits;
      engine.start(limits, gs);
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
