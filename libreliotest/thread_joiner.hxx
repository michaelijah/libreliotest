#pragma once 
#include <vector>
#include <thread>
#include <iostream>


namespace reliotest
{
    using namespace std;
    class thread_joiner
    {
        vector<thread>& threads;

        public: 
        explicit thread_joiner(vector<thread>& threads_): threads(threads_){};
        ~thread_joiner(){
            for(auto& loopthreadref : threads)
            {
                //cout << "thread joined" << endl;
                if(loopthreadref.joinable())
                    loopthreadref.join();
            }
        };
        
    };
};
