#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.emplace_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

float LinuxParser::MemoryUtilization() {
  string memTotal, memFree;
  string line, kB;
  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> memTotal >> memTotal >> kB;
    std::getline(stream, line);
    linestream.clear();
    linestream.str(line);
    linestream >> memFree >> memFree >> kB;
    std::getline(stream, line);
  }
  return (std::stof(memTotal) - std::stof(memFree)) / std::stof(memTotal);
}

long LinuxParser::UpTime() {
  string upTime;
  string line;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> upTime;
  }
  return std::stol(upTime);
}

long LinuxParser::Jiffies() {
  return LinuxParser::IdleJiffies() + LinuxParser::ActiveJiffies();
}

long LinuxParser::ActiveJiffies(int pid) {
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatFilename);
  string token, line;
  vector<string> values{};
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    while (linestream >> token) {
      values.emplace_back(token);
    }
  }
  long utime = stol(values[13]);
  long stime = stol(values[14]);
  long cutime = stol(values[15]);
  long cstime = stol(values[16]);
  return utime + stime + cutime + cstime;
}

long LinuxParser::ActiveJiffies() {
  vector<string> cpuUtilization = CpuUtilization();
  return (stol(cpuUtilization[CPUStates::kUser_]) +
          stol(cpuUtilization[CPUStates::kNice_]) +
          stol(cpuUtilization[CPUStates::kSystem_]) +
          stol(cpuUtilization[CPUStates::kIRQ_]) +
          stol(cpuUtilization[CPUStates::kSoftIRQ_]) +
          stol(cpuUtilization[CPUStates::kSteal_]));
}

long LinuxParser::IdleJiffies() {
  vector<string> cpuUtilization = CpuUtilization();
  return (stol(cpuUtilization[CPUStates::kIdle_]) +
          stol(cpuUtilization[CPUStates::kIOwait_]));
}

vector<string> LinuxParser::CpuUtilization() {
  string token, line;
  vector<string> values;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    while (linestream >> token) {
      if (token == filterCpu) break;
    }
    while (linestream >> token) {
      values.emplace_back(token);
    }
  }
  return values;
}

int LinuxParser::TotalProcesses() {
  return LinuxParser::Pids().size();
}

int LinuxParser::RunningProcesses() {
  string line;
  string value{"-1"};
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::size_t found = line.find(filterRunningProcesses);
      if (found != std::string::npos) {
        value = line.substr(found + 14, found + line.length());
        break;
      }
    }
  }
  return std::stoi(value);
}

string LinuxParser::Command(int pid) {
  string line = "";
  std::ifstream filestream(kProcDirectory + to_string(pid) + kCmdlineFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
  }
  return line;
}

string LinuxParser::Ram(int pid) {
  string line, token;
  long memSize{0L};
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatusFilename);
  while (std::getline(stream, line)) {
    std::istringstream linestream(line);
    while (linestream >> token) {
      if (token == filterProcMem) linestream >> memSize;
    }
  }
  return to_string(memSize * 0.001).substr(0, 6);
}

string LinuxParser::Uid(int pid) {
  string line, token;
  string uid = "";
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatusFilename);
  while (std::getline(stream, line)) {
    std::istringstream linestream(line);
    while (linestream >> token) {
      if (token == filterUID) linestream >> uid;
    }
  }
  return uid;
}

string LinuxParser::User(int pid) {
  string token = Uid(pid);
  string uid;
  string user;
  string line;
  std::ifstream filestream(LinuxParser::kPasswordPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), 'x', ' ');
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> user >> uid) {
        if (token == uid) goto ret;
      }
    }
  }
ret:
  return user;
}

long LinuxParser::UpTime(int pid) {
  long processStartTime;
  string line = "";
  int counter = 22;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kCmdlineFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    while (counter--) {
      linestream >> processStartTime;
    }
  }
  return LinuxParser::UpTime() - (processStartTime / sysconf(_SC_CLK_TCK));
}
