#include "Timer.h"

std::unordered_map<std::string, double> Timer::s_Timings;
std::unordered_map<std::string, size_t> Timer::s_CallCounts;