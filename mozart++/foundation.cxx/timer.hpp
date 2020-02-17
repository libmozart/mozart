/**
 * Mozart++ Template Library: Timer
 * Licensed under MIT License
 * Copyright (c) 2020 Covariant Institute
 * Website: https://covariant.cn/
 * Github:  https://github.com/covariant-institute/
 */

#pragma once

#include <mozart++/core>
#include <chrono>
#include <thread>

namespace mpp {
    class timer final {
        static std::chrono::time_point<std::chrono::high_resolution_clock> m_timer;

    public:
        enum class time_unit {
            nanoseconds,
            microseconds,
            milliseconds,
            seconds,
            minutes
        };

        static void reset() {
            m_timer = std::chrono::high_resolution_clock::now();
        }

        static size_t time(time_unit unit = time_unit::milliseconds) {
            switch (unit) {
                case time_unit::nanoseconds:
                    return std::chrono::duration_cast<std::chrono::nanoseconds>(
                            std::chrono::high_resolution_clock::now() - m_timer)
                            .count();
                case time_unit::microseconds:
                    return std::chrono::duration_cast<std::chrono::microseconds>(
                            std::chrono::high_resolution_clock::now() - m_timer)
                            .count();
                case time_unit::milliseconds:
                    return std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::high_resolution_clock::now() - m_timer)
                            .count();
                case time_unit::seconds:
                    return std::chrono::duration_cast<std::chrono::seconds>(
                            std::chrono::high_resolution_clock::now() - m_timer)
                            .count();
                case time_unit::minutes:
                    return std::chrono::duration_cast<std::chrono::minutes>(
                            std::chrono::high_resolution_clock::now() - m_timer)
                            .count();
            }
            return 0;
        }

        static void delay(size_t time, time_unit unit = time_unit::milliseconds) {
            switch (unit) {
                case time_unit::nanoseconds:
                    std::this_thread::sleep_for(std::chrono::nanoseconds(time));
                    break;
                case time_unit::microseconds:
                    std::this_thread::sleep_for(std::chrono::microseconds(time));
                    break;
                case time_unit::milliseconds:
                    std::this_thread::sleep_for(std::chrono::milliseconds(time));
                    break;
                case time_unit::seconds:
                    std::this_thread::sleep_for(std::chrono::seconds(time));
                    break;
                case time_unit::minutes:
                    std::this_thread::sleep_for(std::chrono::minutes(time));
                    break;
            }
        }

        static size_t measure(const function<void()> &func, time_unit unit = time_unit::milliseconds) {
            size_t begin(0), end(0);
            begin = time(unit);
            func();
            end = time(unit);
            return end - begin;
        }
    };
}
