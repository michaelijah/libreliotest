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
            //Remember Class Members are ctor/dtor like a stack

            //Things that signal  between threads should be deleted last
            std::atomic_bool done; 
            //Threads can be deleted after the work_queue is emptied and dtor'd
            std::vector<thread> threads;
            //work queue should only be destroyed after the joiner join all threads.
            thread_safe_queue<function_wrapper> work_queue;
            //The joiner should fire first to give threads an opportunity to join
            thread_joiner joiner; 

            void worker_thread()
            {

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
                unsigned int const supported_num_of_threads =  std::thread::hardware_concurrency();
                unsigned int const thread_count = (supported_num_of_threads <= 0) ? 12 : supported_num_of_threads;
                threads.reserve(thread_count);

                try
                {
                    for(unsigned int i=0; i < thread_count; ++i)
                    {
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


