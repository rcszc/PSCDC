/* 
* cdc_main_threadpool.
* 顺手用一下原来写的线程池. 
*/
// XCOREserver_threadpool. [PSA.LLM] [ThreadPool_Module]
// RCSZ. version 0.2.0, StartPorj[2023_06_05]
// 
// [2023_06_07] 新增函数功能 Pool Resize.
// [2023_06_08] 修复类不能传入构造参数.
// [2023_07_08] 重写线程池, 使用类实现.

// * [2023_08_07] 使用 [@pomelo_star 2023-2024] XCOREserver
// * [2023_08_07] XCOREserver_threadpool => cdc_main_threadpool

#ifndef _XCORESERVER_THREADPOOL_HPP
#define _XCORESERVER_THREADPOOL_HPP
#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <typeinfo> // RTTI.

#define THPOL_STATE_STOP_POOL   0xE001 // 线程池关闭.
#define THPOL_STATE_FAILED_OBJ  0xE002 // 对象创建失败.
#define THPOL_STATE_FAILED_CRT  0xE003 // 线程创建失败.
#define THPOL_STATE_FAILED_FREE 0xE004 // 线程释放失败.

namespace system_threadpool {

    struct tpc_object {
        std::string create_objectname;
        std::size_t create_objecthash;
    };
    // global variable: current creation objectinfo.
    extern tpc_object create_object;

    template<typename nClass>
    void TaskObjectName(nClass TaskObject) {

        // RunTime Type Identification [RTTI].
        const std::type_info& object_info = typeid(*TaskObject);

        create_object.create_objecthash = object_info.hash_code();
        create_object.create_objectname = object_info.name();
    }
}

// XCOREserver Module Thread_Pool.
namespace ThreadPool {

    size_t getthis_thread_id();

    class xcore_threadpool {
    protected:
        std::vector<std::thread>          thread_workers;
        std::queue<std::function<void()>> pool_tasks;
        std::mutex                        queue_mutex;
        std::condition_variable           workers_condition;

        void __thread_taskexecute(uint32_t num_threads);
        void __thread_taskfree();

        bool _thread_stop_flag = false;

    public:
        xcore_threadpool(uint32_t init_set_threads) {
            __thread_taskexecute(init_set_threads);
        };
        ~xcore_threadpool() {
            __thread_taskfree();
        };

        uint32_t module_state = 0x0001;

        template<typename InClass, typename... Args_param>
        std::future<std::shared_ptr<InClass>> Tp_PushTask(Args_param... args) {
            auto push_task = std::make_shared<std::packaged_task<std::shared_ptr<InClass>()>>(
                [args = std::make_tuple(std::forward<Args_param>(args)...), this]() mutable {
                try {
                    return std::apply([](auto&&... args) {
                        return std::make_shared<InClass>(std::forward<decltype(args)>(args)...);
                    }, std::move(args));
                    module_state = 0x0001;
                }
                catch (...) {
                    module_state = THPOL_STATE_FAILED_OBJ;
                    return std::shared_ptr<InClass>(nullptr);
                }
            });
            system_threadpool::TaskObjectName(push_task);

            std::future<std::shared_ptr<InClass>> _result = push_task->get_future();
            {
                std::unique_lock<std::mutex> _lock(queue_mutex);
                if (_thread_stop_flag) {
                    // disable push task.
                    module_state = THPOL_STATE_STOP_POOL;
                    return _result;
                }
                pool_tasks.emplace([push_task]() { (*push_task)(); });
            }
            workers_condition.notify_one();

            return _result;
        }

        uint32_t Tp_TaskNumber();
        void     Tp_ResizeThreads(uint32_t resize_threads);
    };
}
#define THIS_THREAD_ID ThreadPool::getthis_thread_id

#endif