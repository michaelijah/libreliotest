#include "libreliotest/thread_solution.hxx"
#include <doctest/doctest.h>

#include <fstream>
#include <string>
#include <cstring>

DOCTEST_MAKE_STD_HEADERS_CLEAN_FROM_WARNINGS_ON_WALL_BEGIN
#include <iostream>
DOCTEST_MAKE_STD_HEADERS_CLEAN_FROM_WARNINGS_ON_WALL_END

using namespace reliotest;
using namespace std;

struct read_number_of_lines_input_file
{
    void operator() (std::unique_ptr<taskinfo>& ti)
    {
        //Let's find out how many lines our input file has. That way we can divide read in work between threads
        std::unique_lock<shared_mutex> lock(ti->inputfilemutex);
        ifstream input_file(ti->inputfilename, std::ios::in);
        ti->lines_in_file = 0; 
        string unused;
        while (getline (input_file, unused))
            ++ti->lines_in_file;


        //Can only init the task data after reading in the num of lines in the file
        ti->initialize_task_data();

        //Open the output file, "erase it" and write the change to the file
        //I'm not sure if this works or is even necessary lol
        ofstream output_file(ti->outputfilename, std::ios::out | std::ios::trunc);
        output_file.seekp(0);
        output_file.write("",1);


        return;
    };
};

struct read_input_reverse_lines_into_chunks
{

    void operator() (const size_t &startline,const size_t &chunknumber, const std::unique_ptr<taskinfo>& ti)
    {
        //open the input file in it's own thread dependent ifstream
        ifstream inputfile("input.txt");
        if(!inputfile.is_open()){
            std::cerr << "couldnt open input file" << endl;
            return;
        }


        //name the chunk temporary file based on chunk currently being processed by this functor
        string chunkfilename = "chunk" + std::to_string(chunknumber);
        std::ofstream chunkfilestream(chunkfilename, std::ios::out );
        if(!chunkfilestream.is_open()){
            std::cerr << "couldnt open " << chunkfilename << " file" << endl;
            return;
        }

        string line;
        size_t currentline = 0;

        std::shared_lock<shared_mutex> lock(ti->inputfilemutex);
        while(currentline < startline && getline(inputfile,line))
        {
            //skip ahead until our "read" position matches the requested startline
            ++currentline;
        }

        size_t endline = startline + ti->chunk_size;
        while(currentline < endline  && getline(inputfile,line))
        {
            std::reverse(line.begin(),line.end());
            chunkfilestream << line << endl;
            ++currentline;
        }

    };
};

struct concatenate_chunks_into_output_file
{
    void operator()(const size_t &total_num_chunks)
    {
        //Lets concatenate all of the chunks into our threaded solution output
        std::ofstream output_file("threaded_solution_output.txt", std::ios::out );
        if(!output_file.is_open()){
            std::cerr << "couldnt open output file" << endl;
            return;
        }

        for(size_t chunkedfilenumber = 0; chunkedfilenumber < total_num_chunks ; ++chunkedfilenumber)
        {
            string filename = "chunk" + std::to_string(chunkedfilenumber);
            std::ifstream chunkfilestream(filename,std::ios::binary);
            if(!chunkfilestream.is_open()){
                std::cerr << "couldnt open " << filename << " file" << endl;
                output_file << "***couldnt open " << filename << "***" <<  endl; //embed error in output
                continue; //allow continuation in the event of errors
            }

            //Efficiently append the chunk's contents into the output_file
            output_file << chunkfilestream.rdbuf();
            //Close the chunk file so that we can reuse the ifstream obj
            chunkfilestream.close();

        }

    };
};

TEST_SUITE_BEGIN("implementation" * doctest::description("threaded solution"));
TEST_CASE("Run Threaded Solution")
{
    threaded_moonshot test_object;
    const size_t thread_count = test_object.pool.get_num_threads();

    //create functor
    std::vector<unique_ptr<taskinfo>> task_info_list;
    task_info_list.reserve(1);
    task_info_list.push_back(make_unique<taskinfo>("input.txt","threaded_solution_output.txt", thread_count));



    std::vector<std::future<void>> future_reads;
    std::vector<std::future<void>> future_writes;

    for(auto & ti: task_info_list)
    {
        future_reads.push_back( 
                test_object.pool.submit(
                        [&ti]{
                            auto obj = read_number_of_lines_input_file();
                            obj(ti);
                        }
                    )
                );
    }


    //getting the futures actually "does" the tasks. 
    //Let's do all the reading tasks first
    for(auto& future : future_reads)
        future.get();

    future_reads.clear();


    //Schedule reading the chunks by submitting jobs to our threadpool
    //Each job is given:
    //   "start at this line",
    //   "how many lines should we attempt to read in",
    //   and "what's the current chunk this job is working on"
    size_t current_line_to_read = 0;
    size_t num_chunks_read = 0;

    for(auto & ti: task_info_list)
    {
        current_line_to_read = 0;
        num_chunks_read = 0;
        auto loop_local_total_chunks = ti->total_num_chunks;
        auto loop_local_chunk_size = ti->chunk_size;
        while(num_chunks_read < loop_local_total_chunks)
        {
            future_reads.push_back( 
                    test_object.pool.submit(
                            [current_line_to_read, num_chunks_read,&ti]{
                                auto obj = read_input_reverse_lines_into_chunks();
                                obj(current_line_to_read,num_chunks_read,ti);
                            }
                        )
                    );
            current_line_to_read += loop_local_chunk_size;
            ++num_chunks_read;
        }
    }

    //getting the futures actually "does" the tasks. 
    //Let's do all the reading tasks first
    for(auto& future : future_reads)
        future.get();


    //submit a concatenation job to the thread pool
    for(auto & ti: task_info_list)
    {
        future_writes.push_back( 
                test_object.pool.submit(
                    [&ti]{
                        auto obj = concatenate_chunks_into_output_file();
                        obj(ti->total_num_chunks);
                    }
                    )
                );
    }

    //getting the futures actually "does" the tasks. 
    //Let's do chunk concatenation tasks here
    for(auto& future : future_writes)
        future.get();

}
TEST_SUITE_END();
