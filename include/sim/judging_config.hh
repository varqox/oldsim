#pragma once

#include <chrono>
#include <cstdint>
#include <simlib/concat.hh>
#include <simlib/string_view.hh>

namespace sim {

constexpr unsigned COMPILATION_ERRORS_MAX_LENGTH = 16 << 10; // 32 KiB
constexpr std::chrono::nanoseconds SOLUTION_COMPILATION_TIME_LIMIT = std::chrono::seconds(30);
constexpr uint64_t SOLUTION_COMPILATION_MEMORY_LIMIT = 1 << 30; // 1 GiB
constexpr std::chrono::nanoseconds CHECKER_COMPILATION_TIME_LIMIT = std::chrono::seconds(30);
constexpr uint64_t CHECKER_COMPILATION_MEMORY_LIMIT = 1 << 30; // 1 GiB
// Conver::ResetTimeLimitsOptions and Conver::Options
constexpr std::chrono::nanoseconds MIN_TIME_LIMIT = std::chrono::milliseconds(300);
constexpr std::chrono::nanoseconds MAX_TIME_LIMIT = std::chrono::seconds(22);
constexpr double SOLUTION_RUNTIME_COEFFICIENT = 3;
// JudgeWorker
constexpr std::chrono::nanoseconds CHECKER_TIME_LIMIT = std::chrono::seconds(22);
constexpr uint64_t CHECKER_MEMORY_LIMIT = 512 << 20; // 256 MiB
constexpr double SCORE_CUT_LAMBDA = 2. / 3.; // See JudgeWorker::score_cut_lambda

} // namespace sim
