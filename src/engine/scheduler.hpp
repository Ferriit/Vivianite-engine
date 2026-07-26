#pragma once
#include "time.hpp"
#include "logging.hpp"
#include <vector>
#include <stdexcept>

namespace vivianite {
    namespace Scheduler {
        struct Task {
            enum RUNTYPE {
                ONCE,
                INTERVAL,
                EVERY_FRAME
            };

            enum EXECTYPE {
                ASAP, // As Soon As Possible
                ABS   // Absolute time (only works in ONCE)
            };

            RUNTYPE run_type;
            EXECTYPE exec_type;

            Time::timestamp next_run;
            Time::timestamp interval;
            Time::timestamp delay;
            bool enabled = false;

            // ENGINE REFERENCE ↓
            void (*callback)(void*);

            void* e_ctx;
            
            void execute() {
                if (callback)
                    callback(e_ctx);
            }
        };

        class Scheduler {
            private:
                std::vector<Task> tasks;
            
            public:
                void* e_ctx; // Engine Reference

                void add_task(Task tsk) {
                    tsk.e_ctx = this->e_ctx;
                    
                    if ((tsk.run_type == Task::ONCE) || (tsk.run_type == Task::INTERVAL)) {
                        tsk.next_run = Time().get_time() + tsk.delay;
                    }
                    else if (tsk.exec_type == Task::ASAP) {
                        tsk.next_run = Time().get_time();
                    }

                    if ((tsk.exec_type == Task::ABS) && (tsk.run_type != Task::ONCE)) {
                        Logging().log(Logging::ERROR, "Invalid task type 'ABSOLUTE, NOT-ONCE'");
                        throw std::runtime_error("INVALID ABSOLUTE TASK");
                    }


                }
        };
    };
};
