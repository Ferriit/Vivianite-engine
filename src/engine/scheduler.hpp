#pragma once
#include "time.hpp"
#include "logging.hpp"
#include <vector>

namespace vivianite {
    namespace Scheduler {
        struct Task {
            enum TASK_TYPE {
                ASAP,
                ONCE,
                ABS,
                INTERVAL,
                EVERY_FRAME
            };

            TASK_TYPE run_type;

            Time::timestamp next_run;
            Time::timestamp interval;
            Time::timestamp delay;

            // ENGINE REFERENCE ↓
            void (*callback)(void*, void*);

            void* e_ctx;
            void* r_ctx;
            
            void execute() {
                if (callback)
                    callback(e_ctx, r_ctx);
            }
        };

        class Scheduler {
            private:
                std::vector<Task> tasks;
            
            public:
                void* e_ctx; // Engine Reference
                void* r_ctx; // Renderer Reference

                Logging l_ctx;

                bool running = true;

                void add_task(Task tsk) {
                    tsk.e_ctx = this->e_ctx;
                    tsk.r_ctx = this->r_ctx;

                    l_ctx.log(Logging::DEBUG, "Scheduler: Adding task #%d", tasks.size());
                    
                    if ((tsk.run_type == Task::ONCE) || (tsk.run_type == Task::INTERVAL)) {
                        tsk.next_run = Time().get_time() + tsk.delay;
                    }
                    else if ((tsk.run_type == Task::ASAP) || (tsk.run_type == Task::EVERY_FRAME)) {
                        tsk.next_run = Time().get_time();
                    }

                    else if (tsk.run_type == Task::ABS) {
                        tsk.next_run = tsk.delay;
                    }
                    tasks.push_back(tsk);
                }

                void main_loop() {
                    while (running) {
                        for (size_t i = 0; i < this->tasks.size(); i++) {
                            Task& tsk = this->tasks[i];

                            Time::timestamp now = Time().get_time();

                            if ((now >= tsk.next_run) && (tsk.next_run != -1)) {
                                l_ctx.log(Logging::DEBUG, "Scheduler: Running task #%d", i);

                                if (tsk.run_type == Task::EVERY_FRAME) {
                                    tsk.next_run = now;
                                }
                                else if (tsk.run_type == Task::INTERVAL) {
                                    tsk.next_run += tsk.interval;
                                }
                                else if ((tsk.run_type == Task::ONCE) || (tsk.run_type == Task::ABS) || (tsk.run_type == Task::ASAP)) {
                                    tsk.next_run = -1; // Mark as done
                                }

                                tsk.execute();
                            }
                        }
                    }
                };
        };
    };
};
