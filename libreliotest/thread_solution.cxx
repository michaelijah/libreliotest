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

//template<typename filenametype, typename startpostype, typename chunksizetype>
struct read_input_file_chunk_into_buffer
{


    void operator() (const size_t &startline,const size_t &chunksize, const size_t &chunknumber)
    {
        ifstream inputfile("input.txt");
        if(!inputfile.is_open()){
            std::cerr << "couldnt open input file" << endl;
            return;
        }


        string chunkfilename = "chunk" + std::to_string(chunknumber);
        std::ofstream chunkfilestream(chunkfilename, std::ios::out );
        if(!chunkfilestream.is_open()){
            std::cerr << "couldnt open " << chunkfilename << " file" << endl;
            return;
        }

        string line;
        size_t endline = startline + chunksize;
        size_t currentline = 0;


        //cout << "currentline is " << currentline << endl;
        //cout << "startline is " << startline << endl;
        //cout << "endline is " << endline << endl;
        while(currentline < startline && getline(inputfile,line))
        {
            ++currentline;
        }

        size_t stringelem = 0;
        while(currentline < endline  && getline(inputfile,line))
        {
            std::reverse(line.begin(),line.end());
            chunkfilestream << line << endl;
            ++currentline;
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
    size_t chunk_size = 1000000; //Number of lines to read per task
    const size_t remainder = test_object.lines_in_file % chunk_size;
    const size_t num_whole_chunks = test_object.lines_in_file / chunk_size;
    const size_t total_num_chunks = (remainder > 0) ? num_whole_chunks + 1 : num_whole_chunks;

    std::vector<std::future<void>> futures;


    //Schedule reading the chunks by submitting jobs to our threadpool
    //Each job is given:
    //   "start at this line",
    //   "how many lines should be read in",
    //   and "what's the current chunk this job is working on"
    size_t current_number_of_lines_read = 0;
    size_t chunksreadin = 0;
    while(chunksreadin < num_whole_chunks)
    {
        futures.push_back( 
                test_object.pool.submit(
                        [current_number_of_lines_read, chunk_size, chunksreadin]{
                            auto obj = read_input_file_chunk_into_buffer();
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
        futures.push_back( 
                test_object.pool.submit(
                        [current_number_of_lines_read, remainder, chunksreadin]{
                            auto obj = read_input_file_chunk_into_buffer();
                            obj(current_number_of_lines_read,remainder,chunksreadin);
                        }
                    )
                );
    }



    //getting the futures actually "does" the tasks. 
    for(auto& future : futures)
        future.get();


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

}
TEST_SUITE_END();
