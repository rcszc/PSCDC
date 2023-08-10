// XCOREserver_threadpool.
#include <chrono>

#include "cdc_main_threadpool.hpp"

using namespace std;

namespace system_threadpool {
    tpc_object create_object = {};
}

// PomeloStar ThreadPool Module.
namespace ThreadPool {

    size_t getthis_thread_id() {
        size_t __thread_id = NULL;

        thread::id threadId = std::this_thread::get_id();
        hash<std::thread::id> hasher;
        __thread_id = hasher(threadId);

        return __thread_id;
    }

    void xcore_threadpool::__thread_taskexecute(uint32_t num_threads) {

        // start threads works.
        for (size_t i = 0; i < num_threads; ++i) {
            try {
                thread_workers.emplace_back([this] {

                    // loop execution task.
                    while (true) {
                        function<void()> _work_task;
                        {
                            unique_lock<mutex> _lock(queue_mutex);
                            workers_condition.wait(_lock, [this] { return _thread_stop_flag || !pool_tasks.empty(); });
                            if (_thread_stop_flag && pool_tasks.empty())
                                break;
                            _work_task = move(pool_tasks.front());
                            pool_tasks.pop();
                        }
                        _work_task();
                    }
                });
            }
            catch (...) {
                module_state = THPOL_STATE_FAILED_CRT;
            }
        }
    }

    void xcore_threadpool::__thread_taskfree() {
        {
            unique_lock<mutex> _lock(queue_mutex);
            _thread_stop_flag = true;
        }
        try {
            workers_condition.notify_all();
            for (thread& _worker : thread_workers) {
                // free all threads.
                _worker.join();
            }
        }
        catch (...) {
            module_state = THPOL_STATE_FAILED_FREE;
        }
    }

    uint32_t xcore_threadpool::Tp_TaskNumber() {
        uint32_t _tasks_number = NULL;
        {
            unique_lock<mutex> _lock(queue_mutex);
            _tasks_number = (uint32_t)pool_tasks.size();
        }
        return _tasks_number;
    }

    void xcore_threadpool::Tp_ResizeThreads(uint32_t resize_threads) {
        {
            unique_lock<mutex> _lock(queue_mutex);
            workers_condition.notify_all();
        }
        __thread_taskfree();

        thread_workers.resize(resize_threads);
        _thread_stop_flag = false;

        __thread_taskexecute(resize_threads);
    }
}