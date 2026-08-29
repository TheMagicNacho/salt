#include "defaults.h"

std::string int_to_char(const int input) {
  switch (input) {
      case 1:
          return "A";
      case 2:
          return "B";
      case 3:
          return "C";
      default:
          return " ";
  }
}
