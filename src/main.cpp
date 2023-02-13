#include <iomanip>
#include <iostream>

#include "linux_parser.h"
#include "ncurses_display.h"
#include "system.h"

int main() {
  // std::cout << "OS: " << LinuxParser::OperatingSystem() << '\n';
  // std::cout << "Kernel: " << LinuxParser::Kernel() << '\n';
  // std::cout << "CPU: "; for (auto pid : LinuxParser::Pids()) { std::cout <<
  // pid << " "; }; std::cout << '\n'; std::cout << "Memory: " << std::fixed <<
  // std::setprecision(3)
  //           << LinuxParser::MemoryUtilization() << '\n';
  // std::cout << "CPU Utilization: "; for (auto state :
  // LinuxParser::CpuUtilization()) {std::cout << state << " "; }; std::cout <<
  // '\n'; std::cout << "Running Processes: " << LinuxParser::RunningProcesses()
  // << '\n'; std::cout << "UpTime: " << LinuxParser::UpTime() << '\n';
  // std::cout << LinuxParser::User(3521) << '\n';

  System system;
  NCursesDisplay::Display(system);
}