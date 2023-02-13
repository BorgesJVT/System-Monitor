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
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

float LinuxParser::MemoryUtilization() {
  string memTotal, memFree, memAvailable, buffers, cached;
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
    linestream.clear();
    linestream.str(line);
    linestream >> memAvailable >> memAvailable >> kB;
    std::getline(stream, line);
    linestream.clear();
    linestream.str(line);
    linestream >> buffers >> buffers >> kB;
    std::getline(stream, line);
    linestream.clear();
    linestream.str(line);
    linestream >> cached >> cached >> kB;
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

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  return LinuxParser::IdleJiffies() + LinuxParser::ActiveJiffies();
}

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid) {
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatFilename);
  string token, line;
  vector<string> values{};
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    while (linestream >> token) {
      values.push_back(token);
    }
  }
  long utime = stol(values[13]);
  long stime = stol(values[14]);
  long cutime = stol(values[15]);
  long cstime = stol(values[16]);
  return utime + stime + cutime + cstime;
}

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  vector<string> cpuUtilization = CpuUtilization();
  return (stol(cpuUtilization[CPUStates::kUser_]) +
          stol(cpuUtilization[CPUStates::kNice_]) +
          stol(cpuUtilization[CPUStates::kSystem_]) +
          stol(cpuUtilization[CPUStates::kIRQ_]) +
          stol(cpuUtilization[CPUStates::kSoftIRQ_]) +
          stol(cpuUtilization[CPUStates::kSteal_]));
}

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  vector<string> cpuUtilization = CpuUtilization();
  return (stol(cpuUtilization[CPUStates::kIdle_]) +
          stol(cpuUtilization[CPUStates::kIOwait_]));
}

// TODO: Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  string token, line;
  vector<string> values;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    while (linestream >> token) {
      if (token == "cpu") break;
    }
    while (linestream >> token) {
      values.push_back(token);
    }
  }
  return values;
}

// TODO: Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  return LinuxParser::Pids().size();
  // string line, token;
  // int processes{0};
  // std::ifstream stream(kProcDirectory + kStatFilename);
  // if (stream.is_open()) {
  //   while(std::getline(stream, line)) {
  //     std::istringstream linestream(line);
  //     linestream >> token;
  //     if(token == "processes")
  //       linestream >> processes;
  //   }
  // }
  // return processes;
}

// TODO: Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  string line;
  string value{"-1"};
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::size_t found = line.find("procs_running");
      if (found != std::string::npos) {
        value = line.substr(found + 14, found + line.length());
        break;
      }
    }
  }
  return std::stoi(value);
}

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid) {
  string line = "";
  std::ifstream filestream(kProcDirectory + to_string(pid) + kCmdlineFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
  }
  return line;
}

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid) {
  string line, token;
  long memSize{0L};
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatusFilename);
  while (std::getline(stream, line)) {
    std::istringstream linestream(line);
    while (linestream >> token) {
      if (token == "VmSize:") linestream >> memSize;
    }
  }
  return to_string(memSize * 0.001).substr(0, 6);
}

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid) {
  string line, token;
  string uid = "";
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatusFilename);
  while (std::getline(stream, line)) {
    std::istringstream linestream(line);
    while (linestream >> token) {
      if (token == "Uid:") linestream >> uid;
    }
  }
  return uid;
}

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
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

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
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
