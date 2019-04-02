#include "catch.hpp"

#include <stdexcept>
#include <string>
#include <sstream>
#include "utils.hpp"

TEST_CASE("test converting ints to algebraic notation") {
  CHECK(int_to_algebraic(0)  == "a1");
  CHECK(int_to_algebraic(63) == "h8");
  CHECK(int_to_algebraic(20) == "e3");
  CHECK(int_to_algebraic(41) == "b6");
  CHECK(int_to_algebraic(10) == "c2");
  CHECK(int_to_algebraic(57) == "b8");
}

TEST_CASE("test converting algebraic notation to int") {
  CHECK(algebraic_to_int("a1") == 0 );
  CHECK(algebraic_to_int("h8") == 63);
  CHECK(algebraic_to_int("f4") == 29);
  CHECK(algebraic_to_int("h2") == 15);
  CHECK(algebraic_to_int("d7") == 51);
  CHECK(algebraic_to_int("g3") == 22);
}

TEST_CASE("int_to_algebraic fails on out of bound input") {
  CHECK_THROWS_AS(int_to_algebraic(-1), std::domain_error);
  CHECK_THROWS_AS(int_to_algebraic(64), std::domain_error);
}

class ErrorMessageContains : public Catch::MatcherBase<std::exception> {
  private:
    std::string contained;

  public:
    ErrorMessageContains(std::string str) : contained(str) {}
    virtual bool match(std::exception const& e) const override {
      return std::string(e.what()).find(contained) != std::string::npos;
    }
    virtual std::string describe() const override {
      std::ostringstream oss;
      oss << "error message contains '" << contained << "'";
      return oss.str();
    }
};

inline ErrorMessageContains ErrContains(std::string c) {
  return ErrorMessageContains(c);
}

TEST_CASE("algebraic_to_int fails on out of bound input") {
  CHECK_THROWS_MATCHES(algebraic_to_int("Z1"), std::domain_error, ErrContains("file"));
  CHECK_THROWS_MATCHES(algebraic_to_int("i8"), std::domain_error, ErrContains("file"));
  CHECK_THROWS_MATCHES(algebraic_to_int("a0"), std::domain_error, ErrContains("rank"));
  CHECK_THROWS_MATCHES(algebraic_to_int("h9"), std::domain_error, ErrContains("rank"));
}
