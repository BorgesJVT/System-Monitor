#include <iomanip>
#include <iostream>

#include "linux_parser.h"
#include "ncurses_display.h"
#include "system.h"

int main() {
  std::cout << "OS: " << LinuxParser::OperatingSystem() << '\n';
  std::cout << "Kernel: " << LinuxParser::Kernel() << '\n';
  std::cout << "Memory Utilization: " << std::fixed << std::setprecision(3)
            << LinuxParser::MemoryUtilization() << '\n';
  std::cout << "UpTime: " << LinuxParser::UpTime() << '\n';

  // System system;
  // NCursesDisplay::Display(system);
}