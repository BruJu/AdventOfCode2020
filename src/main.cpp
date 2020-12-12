#include <iostream>
#include "framework/configuration.h"
#include <algorithm>
#include "colors.h"

using DayEntryPoint = Output(const std::vector<std::string> & lines);

Output day01(const std::vector<std::string> & lines);
Output day02(const std::vector<std::string> & lines);
Output day03(const std::vector<std::string> & lines);
Output day04(const std::vector<std::string> & lines);
Output day05(const std::vector<std::string> & lines);
Output day06(const std::vector<std::string> & lines);
Output day07(const std::vector<std::string> & lines);
Output day08(const std::vector<std::string> & lines);
Output day09(const std::vector<std::string> & lines);
Output day10(const std::vector<std::string> & lines);
Output day11(const std::vector<std::string> & lines);


void dispatch(const InputConfig & config, TestScore & ts) {
    static constexpr std::array<DayEntryPoint *, 30> days {
        &day01, &day02, &day03, &day04, &day05, &day06, &day07,
        &day08, &day09, &day10, &day11, nullptr
    };

    if (DayEntryPoint * day = days[config.day - 1]) {
        ts += config.run(day);
    } else {
        std::cout << "\x1B[1m" KRED "-- No handler for day " << config.day
                << ": Required for " << config.filename << RST "\n";
    }
}

int main(int argc, const char * argv[]) {
    auto configs = InputConfig::read_configuration();

    int required_day;
    if (argc <= 1) {
        required_day = std::max_element(configs.begin(), configs.end(),
            [](const auto & lhs, const auto & rhs) { return lhs.day < rhs.day; }
        )->day;
    } else {
        required_day = std::stoi(argv[1]);
    }

    TestScore testScore;

    std::optional<int> last_seen_day = std::nullopt;

    // TODO : display ordered by day > part instead of day > files

    for (const auto & config : configs) {
        if (config.day == required_day || required_day == 0) {
            if (last_seen_day.has_value() && config.day != *last_seen_day) {
                std::cout << "\n";
            }
            last_seen_day = config.day;

            dispatch(config, testScore);
        }
    }

    std::cout << "\x1B[1m"
              << "\nTotal  = " << testScore.total()
              << "\n" KGRN "Success= " << testScore.success
              << "\n" KRED "Failed = " << testScore.failed << RST << '\n';

    return 0;
}
