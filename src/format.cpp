#include "format.h"

#include <iomanip>
#include <sstream>
#include <string>

using std::string;

// TODO: Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
// REMOVE: [[maybe_unused]] once you define the function
string Format::ElapsedTime(long seconds) {
  long minutes = seconds / 60.0;
  int hours = minutes / 60.0;
  minutes = minutes % 60;
  int remaining_seconds = seconds % 60;

  // std::ostream& os;
  std::ostringstream oss;
  oss << std::setfill('0') << std::setw(2) << std::to_string(hours) << ':'
      << std::setfill('0') << std::setw(2) << std::to_string(minutes) << ':'
      << std::setfill('0') << std::setw(2) << std::to_string(remaining_seconds)
      << '.';
  return oss.str();
}