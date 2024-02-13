#pragma once
#include <thread>
#include "function_wrapper.hxx"
#include "thread_safe_queue.hxx"
#include "thread_joiner.hxx"
#include <atomic>
#include <future>

namespace reliotest
{
    class thread_pool
    {
        private:
            std::atomic_bool done;
            std::vector<thread> threads;
            thread_joiner joiner;
            thread_safe_queue<function_wrapper> work_queue;

            void worker_thread()
            {
                //cout << "pushback succeeded" << endl;

                while(!done)
                {
                    function_wrapper task;

                    if(work_queue.try_pop(task))
                        task();
                    else
                        this_thread::yield();
                }
            };




        public:

            thread_pool() : done(false), joiner(threads)
            {
                unsigned int const thread_count = std::thread::hardware_concurrency();
                threads.reserve(thread_count);

                //std::cout << "thread_count " << thread_count << std::endl;
                try
                {
                    for(unsigned int i=0; i < thread_count; ++i)
                    {
                        //cout << "try pushing back a thread" << endl;
                        threads.push_back(std::thread(&thread_pool::worker_thread,this));
                    }
                }
                catch(...)
                {
                    done = true;
                    throw;
                }
            };

            ~thread_pool()
            {
                done = true;
            };

            template<typename FunctionType>
            std::future<typename std::result_of<FunctionType()>::type> submit(FunctionType f)
            {
                typedef typename std::result_of<FunctionType()>::type 
                    result_type;

                std::packaged_task<result_type()> task(std::move(f));
                std::future<result_type> res(task.get_future());
                work_queue.push(std::move(task));
                return res;
            };

            size_t get_num_threads(){return threads.size();};
    };
};


