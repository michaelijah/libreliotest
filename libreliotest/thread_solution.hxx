#pragma once
#include <string>
#include <stdexcept>
#include <iostream>
#include <fstream>

//threaded support structure included RAII thread_joiner, function_wrapper, thread_pool and thread_safe_queue
#include <thread>
#include <atomic>
#include <future>
#include <libreliotest/thread_pool.hxx>


namespace reliotest
{
    using namespace std;
    struct threaded_moonshot
    {
        string inputfilename;
        size_t lines_in_file;
        string outputfilename;


        thread_pool pool;

        threaded_moonshot(string in_name, string out_name) : inputfilename(in_name),outputfilename(out_name)
        {
            //Let's find out how many lines our input file has. That way we can divide read in work between threads
            ifstream input_file(inputfilename, std::ios::in);
            lines_in_file = 0; 
            string unused;
            while (getline (input_file, unused))
                ++lines_in_file;

            //Open the output file, "erase it" and write the change to the file
            //I'm not sure if this works or is even necessary lol
            ofstream output_file(outputfilename, std::ios::out | std::ios::trunc);
            output_file.seekp(0);
            output_file.write("",1);

        };
    };
};
