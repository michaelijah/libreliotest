#pragma once
#include <string>
#include <stdexcept>
#include <iostream>
#include <fstream>

//threaded support structure included RAII thread_joiner, function_wrapper, thread_pool and thread_safe_queue
#include <thread>
#include <atomic>
#include <future>
#include <shared_mutex>
#include <libreliotest/thread_pool.hxx>


namespace reliotest
{
    using namespace std;

    struct taskinfo
    {
        string inputfilename;
        shared_mutex inputfilemutex;
        string outputfilename;
        shared_mutex outputfilemutex;
        size_t thread_count;
        size_t lines_in_file;
        size_t chunk_size; //Number of lines to read per task
        size_t remainder;
        size_t num_whole_chunks;
        size_t total_num_chunks;

        taskinfo(string in_name, string out_name,size_t available_threads_for_task) :
        inputfilename(in_name),outputfilename(out_name),thread_count(available_threads_for_task){};

       void initialize_task_data() {
            chunk_size = lines_in_file / thread_count; //Number of lines to read per task
            remainder = lines_in_file % chunk_size;
            num_whole_chunks = lines_in_file / chunk_size;
            total_num_chunks = (remainder > 0) ? num_whole_chunks + 1 : num_whole_chunks;
        };
    };

    struct threaded_moonshot
    {
        thread_pool pool;

        threaded_moonshot() {};
    };
};
