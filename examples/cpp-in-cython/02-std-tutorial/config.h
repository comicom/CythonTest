#ifndef EXAMPLE_COMMON_PROCESS_CONFIG_H_
#define EXAMPLE_COMMON_PROCESS_CONFIG_H_

#include <iostream>
#include <fstream>
#include <atomic>
#include <string>
#include <map>
#include <vector>

using namespace std;
typedef map<string, vector<string>> config;

namespace example
{
namespace common
{
namespace process
{
    class ConfParcer
    {    
        public:
            // Constructor & Deconstructor
            ConfParcer() = delete;
            ConfParcer(istream &in);
            // ~ConfParcer();
            // Process config file
            string trim(const string &s);
            config setBlock(istream &in);
            config setCountext(vector<string> &in);
            void setIP(const vector<string> &in);
            void setTCPPort(const vector<string> &in);
            void setUDPPort(const vector<string> &in);
            void setLevel(const vector<string> &in);
            void setMode(const vector<string> &in);
            // static casting
            uint16_t str2uint16(string in);
            uint8_t str2uint8(string in);
            // API
            string getIP();
            uint16_t getTCPPort();
            uint16_t getUDPPort();
            string getLevel();
            uint8_t getMode();
        private:
            // init value
            atomic<bool> mInitiated {false};
            // structure for config file
            config mBlock;
            config mCountext;
            // member variable
            std::string mIP;
            uint16_t mTCPPort;
            uint16_t mUDPPort;
            std::string mLevel;
            uint8_t mMode;
    };
}
}
}

#endif