#pragma once
#include <chrono>
#include <ctime>

namespace vivianite {
    class Time {
    public:
        struct date {
            int year;
            int month;
            int day;
        };

        struct clock {
            int hour;
            int min;
            int sec;
        };

        struct date_time {
            int year;
            int month;
            int day;
            int hour;
            int min;
            int sec;
            int millisec;
        };

        float delta_time = 0.0f;

        inline int64_t timestamp() {
            auto now = std::chrono::system_clock::now();

            return std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()
            ).count();
        }

        inline date get_date_UTC() {
            auto now = std::chrono::system_clock::now();
            std::time_t t = std::chrono::system_clock::to_time_t(now);

            std::tm utc = *std::gmtime(&t);

            return {
                .year = utc.tm_year + 1900,
                .month = utc.tm_mon + 1,
                .day = utc.tm_mday
            };
        }

        inline clock get_clock_UTC() {
            auto now = std::chrono::system_clock::now();
            std::time_t t = std::chrono::system_clock::to_time_t(now);

            std::tm utc = *std::gmtime(&t);

            return {
                .hour = utc.tm_hour,
                .min = utc.tm_min,
                .sec = utc.tm_sec
            };
        }

        inline date_time get_date_time_UTC() {
            auto now = std::chrono::system_clock::now();

            auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()
            ).count();

            std::time_t t = std::chrono::system_clock::to_time_t(now);
            std::tm utc = *std::gmtime(&t);

            return {
                .year = utc.tm_year + 1900,
                .month = utc.tm_mon + 1,
                .day = utc.tm_mday,
                .hour = utc.tm_hour,
                .min = utc.tm_min,
                .sec = utc.tm_sec,
                .millisec = static_cast<int>(millis % 1000)
            };
        }

        inline int get_timezone_offset() {
            std::time_t now = std::time(nullptr);

            std::tm local = *std::localtime(&now);
            std::tm utc = *std::gmtime(&now);

            int offset = static_cast<int>(
                std::difftime(std::mktime(&local), std::mktime(&utc))
            );

            return offset;
        }
    };
}
