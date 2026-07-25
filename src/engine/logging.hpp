#pragma once
#include "time.hpp"
#include <cstdarg>
#include <cstdio>

namespace vivianite {
    class Logging {
        public:
        enum log_level {
            INFO,
            WARNING,
            ERROR,
            FATAL,
            NOTICE,
            UNMARKED
        };

        bool colored;

        Logging(bool colored = true)
            : colored(colored) {}

        inline void log(log_level level, const char* format, ...) {
            Time::timestamp dt_UTC = Time().get_time();
            Time::date_time dt = Time().utc_to_date_time(dt_UTC);

            const char* colors[] = {
                "\x1b[32m",
                "\x1b[28;5;214m",
                "\x1b[31;1m",
                "\x1b[31;40m",
                "\x1b[34;1m",
                "\x1b[0m"
            };

            printf("%s", colors[level]);
            if (!colored)
                printf("\x1b[0m");
            
            switch (level) {
                case log_level::INFO:
                    printf("[INFO] ");
                    break;
                case log_level::WARNING:
                    printf("[WARN] ");
                    break;
                case log_level::ERROR:
                    printf("[ERROR] ");
                    break;
                case log_level::FATAL:
                    printf("[FATAL]");
                    break;
                case log_level::NOTICE:
                    printf("[NOTICE] ");
                    break;
                case log_level::UNMARKED:
                    break;
            }

            printf(
                "[%04d-%02d-%02d %02d:%02d:%02d] ",
                dt.year,
                dt.month,
                dt.day,
                dt.hour,
                dt.min,
                dt.sec
            );

            va_list args;
            va_start(args, format);
            vprintf(format, args);
            va_end(args);

            printf("\n");

            fflush(stdout);
        }
    };
};
