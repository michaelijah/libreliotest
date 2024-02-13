#include <string>
#include <stdexcept>
#include <iostream>
#include <fstream>
namespace reliotest{
    using namespace std;
    struct naivety
    {
        string inputfilename; 
        string outputfilename; 
        naivety(string in_name, string out_name) : inputfilename(in_name),outputfilename(out_name){};

        void operator()() {
            ifstream input_file(inputfilename);
            ofstream output_file(outputfilename);
            if(!input_file.is_open() | !output_file.is_open()){
                std::cerr << "Failed to open input or output file" << endl;
                throw runtime_error("Failed to open input or output file" );
            }

            std::string line;
            while (getline(input_file,line))
            {
                std::reverse(line.begin(),line.end());
                output_file << line << endl;
            }
        };
    };

}
