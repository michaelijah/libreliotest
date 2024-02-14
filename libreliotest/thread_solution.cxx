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

struct read_input_reverse_lines_into_chunks
{

    void operator() (const size_t &startline,const size_t &chunksize, const size_t &chunknumber)
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

        while(currentline < startline && getline(inputfile,line))
        {
            //skip ahead until our "read" position matches the requested startline
            ++currentline;
        }

        size_t endline = startline + chunksize;
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
    void operator()(size_t total_num_chunks)
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
    //create functor
    threaded_moonshot test_object("input.txt","threaded_solution_output.txt");

    const size_t thread_count = test_object.pool.get_num_threads();
    const size_t max_number_lines_in_file = test_object.lines_in_file;
    const size_t chunk_size = 1000000; //Number of lines to read per task
    const size_t remainder = test_object.lines_in_file % chunk_size;
    const size_t num_whole_chunks = test_object.lines_in_file / chunk_size;
    const size_t total_num_chunks = (remainder > 0) ? num_whole_chunks + 1 : num_whole_chunks;

    std::vector<std::future<void>> future_reads;
    std::vector<std::future<void>> future_writes;


    //Schedule reading the chunks by submitting jobs to our threadpool
    //Each job is given:
    //   "start at this line",
    //   "how many lines should be read in",
    //   and "what's the current chunk this job is working on"
    size_t current_number_of_lines_read = 0;
    size_t chunksreadin = 0;
    while(chunksreadin < num_whole_chunks)
    {
        future_reads.push_back( 
                test_object.pool.submit(
                        [current_number_of_lines_read, chunk_size, chunksreadin]{
                            auto obj = read_input_reverse_lines_into_chunks();
                            obj(current_number_of_lines_read,chunk_size,chunksreadin);
                        }
                    )
                );
        current_number_of_lines_read += chunk_size;
        ++chunksreadin;
    }

    //I'm sure theres a better way to do this. maybe Do-while or modify the above while loop
    if(remainder > 0)
    {
        future_reads.push_back( 
                test_object.pool.submit(
                        [current_number_of_lines_read, remainder, chunksreadin]{
                            auto obj = read_input_reverse_lines_into_chunks();
                            obj(current_number_of_lines_read,remainder,chunksreadin);
                        }
                    )
                );
    }



    //getting the futures actually "does" the tasks. 
    //Let's do all the reading tasks first
    for(auto& future : future_reads)
        future.get();


    //submit a concatenation job to the thread pool
    future_writes.push_back( 
            test_object.pool.submit(
                [=]{
                auto obj = concatenate_chunks_into_output_file();
                obj(total_num_chunks);
                }
                )
            );

    //getting the futures actually "does" the tasks. 
    //Let's do chunk concatenation tasks here
    for(auto& future : future_writes)
        future.get();

}
TEST_SUITE_END();
