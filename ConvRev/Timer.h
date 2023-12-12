// basic Timer class for profiling
// source: https://github.com/oschonrock/toolbelt/blob/master/os/bch.hpp

#pragma once

#include <chrono>
#include <cstdio>
#include <iostream>
#include <string>
#include <utility>
#include <unordered_map>

using clk        = std::chrono::high_resolution_clock;
using time_point = std::chrono::time_point<clk>;
using std::chrono::duration;
using std::chrono::duration_cast;

class Timer {
public:
  explicit Timer(std::string label_, size_t freq = 1) 
    : start_{clk::now()}, label{std::move(label_)}, printFreq{freq}
  {}

  Timer(const Timer& other) = default;
  Timer& operator=(const Timer& other) = default;

  Timer(Timer&& other) = default;
  Timer& operator=(Timer&& other) = default;

  ~Timer() { print(); }

  void print() {
    finish_         = clk::now();
    auto elapsed    = finish_ - start_;
    auto elapsed_s  = duration_cast<duration<double>>(elapsed).count();
    auto elapsed_ms = elapsed_s * 1000;
    char buf[100]{0};                                                // NOLINT

    // update average
    s_Timings[label] += elapsed_ms;

    // bump call count
    s_CallCounts[label]++;

    // print if freq is met
    if (s_CallCounts[label] % printFreq == 0) {
      // print average
      std::snprintf(buf, 100, "%-20s %12.4f ms", label.data(), s_Timings[label] / s_CallCounts[label]); // NOLINT
      std::cerr << buf << '\n';                                       // NOLINT
    }


  }

public: 
    // map for storing cumulative timings
    static std::unordered_map<std::string, double> s_Timings;
    // map for storing call counts
    static std::unordered_map<std::string, size_t> s_CallCounts;

private:
  time_point  start_;
  time_point  finish_;
  std::string label;
  size_t      printFreq;
};
