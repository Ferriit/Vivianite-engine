#include "include.hpp"

namespace vivianite {
    namespace Scheduler {
        void Task::execute() {
            if (callback)
                callback();
        }
        
        void Scheduler::add_task(Task tsk) {
            tsk.r_ctx = this->r_ctx;

            l_ctx->log(Logging::INFO, "Scheduler: Adding task #{}", tasks.size());

            if ((tsk.run_type == Task::EVERY_FRAME) && (tsk.multi_threaded)) {
                l_ctx->log(Logging::WARNING, "Unable to add task #{}, as it's EVERY_FRAME, multi_threaded", tasks.size());
                return;
            }
            
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

        void Scheduler::main_loop() {
            std::thread t_loop([this]() {this->thread_loop();});

            while (running) {
                for (size_t i = 0; i < this->tasks.size(); i++) {
                    Task& tsk = this->tasks[i];

                    if (tsk.multi_threaded)
                        continue;

                    Time::timestamp now = Time().get_time();

                    if ((now >= tsk.next_run) && (tsk.next_run != -1)) {
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

            t_loop.join();
        };

        void Scheduler::thread_loop() {
            while (running) {
                for (size_t i = 0; i < this->tasks.size(); i++) {
                    Task& tsk = this->tasks[i];

                    if (!tsk.multi_threaded)
                        continue;

                    Time::timestamp now = Time().get_time();

                    if ((now >= tsk.next_run) && (tsk.next_run != -1)) {
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
        }
    };
};
