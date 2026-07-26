#pragma once
#include <chrono>
#include <ctime>

namespace vivianite {
    class Time {
        public:
        using timestamp = long long;

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

        inline timestamp get_time() {
            auto now = std::chrono::system_clock::now();

            return std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()
            ).count();
        }

        inline date get_date_utc() {
            auto now = std::chrono::system_clock::now();
            std::time_t t = std::chrono::system_clock::to_time_t(now);

            std::tm utc = *std::gmtime(&t);

            return {
                .year = utc.tm_year + 1900,
                .month = utc.tm_mon + 1,
                .day = utc.tm_mday
            };
        }

        inline clock get_clock_utc() {
            auto now = std::chrono::system_clock::now();
            std::time_t t = std::chrono::system_clock::to_time_t(now);

            std::tm utc = *std::gmtime(&t);

            return {
                .hour = utc.tm_hour,
                .min = utc.tm_min,
                .sec = utc.tm_sec
            };
        }

        inline date_time get_date_time_utc() {
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

        inline timestamp get_timezone_offset() {
            std::time_t now = std::time(nullptr);

            std::tm local = *std::localtime(&now);
            std::tm utc = *std::gmtime(&now);

            int offset = static_cast<int>(
                std::difftime(std::mktime(&local), now)
            );

            return offset;
        }

        // utc timestamp -> local date
        inline date utc_to_date(timestamp ts) {
            ts += get_timezone_offset() * 1000;

            std::time_t time = ts / 1000;
            std::tm result = *std::gmtime(&time);

            return {
                result.tm_year + 1900,
                result.tm_mon + 1,
                result.tm_mday
            };
        }


        // utc timestamp -> local clock
        inline clock utc_to_clock(timestamp ts) {
            ts += get_timezone_offset() * 1000;

            std::time_t time = ts / 1000;
            std::tm result = *std::gmtime(&time);

            return {
                result.tm_hour,
                result.tm_min,
                result.tm_sec
            };
        }


        // utc timestamp -> local date_time
        inline date_time utc_to_date_time(timestamp ts) {
            std::time_t time = ts / 1000;

            std::tm result = *std::localtime(&time);

            return {
                result.tm_year + 1900,
                result.tm_mon + 1,
                result.tm_mday,
                result.tm_hour,
                result.tm_min,
                result.tm_sec,
                static_cast<int>(ts % 1000)
            };
        }


        // local date -> utc timestamp
        inline timestamp date_to_utc(date d) {
            std::tm tm = {};
            tm.tm_year = d.year - 1900;
            tm.tm_mon = d.month - 1;
            tm.tm_mday = d.day;

            return static_cast<timestamp>(
                std::mktime(&tm)
            ) * 1000 - get_timezone_offset() * 1000;
        }


        // local clock -> utc timestamp
        inline timestamp clock_to_utc(clock c) {
            std::tm tm = {};
            tm.tm_hour = c.hour;
            tm.tm_min = c.min;
            tm.tm_sec = c.sec;

            return static_cast<timestamp>(
                std::mktime(&tm)
            ) * 1000 - get_timezone_offset() * 1000;
        }


        // local date_time -> utc timestamp
        inline timestamp date_time_to_utc(date_time dt) {
            std::tm tm = {};

            tm.tm_year = dt.year - 1900;
            tm.tm_mon = dt.month - 1;
            tm.tm_mday = dt.day;

            tm.tm_hour = dt.hour;
            tm.tm_min = dt.min;
            tm.tm_sec = dt.sec;

            return static_cast<timestamp>(
                std::mktime(&tm)
            ) * 1000 - get_timezone_offset() * 1000
                + dt.millisec;
        }
    };
}
