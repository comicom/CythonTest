#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iterator>
#include <iostream>
#include <sstream>
#include <map>
#include <string>
#include <vector>

#include "config.h"

using namespace std;

example::common::process::ConfParcer::ConfParcer(istream &in){
    // setup 
    mBlock = example::common::process::ConfParcer::setBlock(in);
    // setup countext
    config address = example::common::process::ConfParcer::setCountext(mBlock["ADDRESS"]);
    config log = example::common::process::ConfParcer::setCountext(mBlock["LOG"]);
    // setup member variable
    // const vector<string> &mBAddress = mBlock["ADDRESS"];
    // cout << "ADDRESS:\n";
    // std::copy(mBAddress.begin(), mBAddress.end(), ostream_iterator<string>(cout, "\n"));
    const vector<string> &ip = address["IP"];
    const vector<string> &tcp = address["TCP_PORT"];
    const vector<string> &udp = address["UDP_PORT"];
    const vector<string> &level = log["LEVEL"];
    const vector<string> &mode = log["MODE"];

    example::common::process::ConfParcer::setIP(ip);
    example::common::process::ConfParcer::setTCPPort(tcp);
    example::common::process::ConfParcer::setUDPPort(udp);
    example::common::process::ConfParcer::setLevel(level);
    example::common::process::ConfParcer::setMode(mode);
    mInitiated = true;
}

string example::common::process::ConfParcer::trim(const string &s)
{
    string::const_iterator front = find_if(s.begin(), s.end(), not1(ptr_fun(::isspace)));
    string::const_iterator back = find_if(s.rbegin(), s.rend(), not1(ptr_fun(::isspace))).base();
    return (back < front) ? string() : string(front, back);
}

config example::common::process::ConfParcer::setBlock(istream &in){
    // TODO: add support for comments
    config block;
    string section; // current section
    string line; // current line
    while (getline(in, line))
    {
        line = example::common::process::ConfParcer::trim(line);
        if (line.empty()) continue;

        if (line[0] == '[' && *line.rbegin() == ']')
        {
            // This line looks like it's starting a new section
            string newsection = example::common::process::ConfParcer::trim(line.substr(1, line.size() - 2));
            if (newsection.empty())
            {
                std::cerr << "warning: empty section found\n";
                continue;
            }
        section = newsection;
        }
        else
        block[section].push_back(line);
    }
    return block;
}

config example::common::process::ConfParcer::setCountext(vector<string> &in){
    // TODO: add support for comments
    config countext = {};
    string delimiter = "=";
    string word;
    vector<string> words;
    size_t pos;
    for (int lines=0; lines<in.size(); lines++)
    {
        words = {};
        word = example::common::process::ConfParcer::trim(in[lines]);
        pos = word.find(delimiter);
        words.push_back(word.substr(0, pos));
        words.push_back(word.substr(pos+1,word.size()));
        countext[words[0]].push_back(words[1]);
    }
    return countext;
}

void example::common::process::ConfParcer::setIP(const vector<string> &in)
{
    stringstream result;
    copy(in.begin(), in.end(), ostream_iterator<string>(result, ""));
    mIP = result.str();
}

void example::common::process::ConfParcer::setTCPPort(const vector<string> &in)
{
    stringstream result;
    copy(in.begin(), in.end(), ostream_iterator<string>(result, ""));
    mTCPPort = example::common::process::ConfParcer::str2uint16(result.str());    // mTCPPort = address["TCP_PORT"];
}

void example::common::process::ConfParcer::setUDPPort(const vector<string> &in)
{
    stringstream result;
    copy(in.begin(), in.end(), ostream_iterator<string>(result, ""));
    mUDPPort = example::common::process::ConfParcer::str2uint16(result.str());
    // mUDPPort = address["UDP_PORT"];
}

void example::common::process::ConfParcer::setLevel(const vector<string> &in)
{
    stringstream result;
    copy(in.begin(), in.end(), ostream_iterator<string>(result, ""));
    mLevel = result.str();
}

void example::common::process::ConfParcer::setMode(const vector<string> &in)
{
    stringstream result;
    copy(in.begin(), in.end(), ostream_iterator<string>(result, ""));
    mMode = example::common::process::ConfParcer::str2uint8(result.str());
}

uint16_t example::common::process::ConfParcer::str2uint16(string in)
{
    int myInt(stoi(in));
    uint16_t myUInt16(0);
    if (myInt <= static_cast<int>(UINT16_MAX) && myInt >=0) {
        myUInt16 = static_cast<uint16_t>(myInt);
    }
    else {
        std::cout << "Error : Manage your error the way you want to\n";
    }
    return myUInt16;
}

uint8_t example::common::process::ConfParcer::str2uint8(string in)
{
    int myInt(stoi(in));
    uint8_t myUInt8(0);
    if (myInt <= static_cast<int>(UINT8_MAX) && myInt >=0) {
        myUInt8 = static_cast<uint16_t>(myInt);
    }
    else {
        std::cout << "Error : Manage your error the way you want to\n";
    }
    return myUInt8;
}

string example::common::process::ConfParcer::getIP()
{
    return mIP;
}

uint16_t example::common::process::ConfParcer::getTCPPort()
{
    return mTCPPort;
}

uint16_t example::common::process::ConfParcer::getUDPPort()
{
    return mUDPPort;
}

string example::common::process::ConfParcer::getLevel()
{
    return mLevel;
}

uint8_t example::common::process::ConfParcer::getMode()
{
    return mMode;
}