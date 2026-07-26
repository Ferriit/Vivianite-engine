#pragma once
#include "time.hpp"
#include <cstdarg>
#include <cstdio>
#include <string>
#include <vector>

namespace vivianite {
    class Logging {
        public:
        enum log_level {
            INFO,
            WARNING,
            ERROR,
            FATAL,
            NOTICE,
            DEBUG,
            UNMARKED,
        };

        struct log_entry {
            log_level level;
            Time::date_time timestamp;
            std::string message;
        };

        bool colored;

        bool verbose = false;

        std::vector<log_entry> buffer;

        size_t max_buffer_size = 1000;

        Logging(bool colored = true)
            : colored(colored) {}

        inline void log(log_level level, const char* format, ...) {
            if ((level == log_level::DEBUG) && !verbose) {
                return;
            }

            Time::timestamp dt_UTC = Time().get_time();
            Time::date_time dt = Time().utc_to_date_time(dt_UTC);

            // Format message
            char message[1024];

            va_list args;
            va_start(args, format);
            vsnprintf(message, sizeof(message), format, args);
            va_end(args);

            // Store in buffer
            buffer.push_back({
                level,
                dt,
                std::string(message)
            });

            if (buffer.size() > max_buffer_size)
                buffer.erase(buffer.begin());


            const char* colors[] = {
                "\x1b[32m",
                "\x1b[38;5;214m",
                "\x1b[31;1m",
                "\x1b[31;40m",
                "\x1b[34;1m",
                "\x1b[38;5;242m",
                "\x1b[0m"
            };

            if (colored)
                printf("%s", colors[level]);

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
                    printf("[FATAL] ");
                    break;
                case log_level::NOTICE:
                    printf("[NOTICE] ");
                    break;
                case log_level::DEBUG:
                    printf("[DEBUG] ");
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

            if (colored)
                printf("\x1b[0m");

            printf("%s\n", message);

            fflush(stdout);
        }

        inline void clear() {
            buffer.clear();
        }
    };
};
