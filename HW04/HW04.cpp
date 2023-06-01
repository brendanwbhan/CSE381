// Copyright 2023 Brendan Han

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <utility>
#include <vector>
#include <algorithm>
#include <numeric>
#include <unordered_map>
#include <boost/asio.hpp>
#include <stdexcept>

#include "ChildProcess.h"

// It is ok to use the following namespace delarations in C++ source
// files only. They must never be used in header files.
using namespace boost::asio;
using namespace boost::asio::ip;
using namespace std;

void processUrl(std::istream& is);
void readUrl(std::string url);
std::vector<std::string> stringToVec(std::string str);
void process(std::istream& is, const std::string& prompt);

/**
 * Helper method to setup a TCP stream for downloading data from an
 * web-server.
 * 
 * @param host The host name of the web-server. Host names can be of
 * the from "www.miamioh.edu" or "ceclnx01.cec.miamioh.edu".  This
 * information is typically extracted from a given URL.
 *
 * @param path The path to the file being download.  An example of
 * this value is ""
 *
 * @param socket The TCP stream (aka socket) to be setup by this
 * method.  After a successful call to this method, this stream will
 * contain the response from the web-server to be processed.
 *
 * @param port An optional port number. The default port number is "80"
 *
 */
void setupDownload(const std::string& hostName, const std::string& path,
                   tcp::iostream& data, const std::string& port = "80") {
    // Create a boost socket and request the log file from the server.
    data.connect(hostName, port);
    data << "GET "   << path     << " HTTP/1.1\r\n"
         << "Host: " << hostName << "\r\n"
         << "Connection: Close\r\n\r\n";
}

/**
 * A helper method that puts a vector together into a string
 *
 * @param vec the vector we want to put together
 * 
 * @return the string version of the vector
 */
std::string putTogether(vector<std::string> vec) {
    std::string ret;
    int size = vec.size();
    for (int i = 0; i < size; i++) {
        ret = ret + vec[i] + " ";
    }
    ret = ret.substr(0, ret.length() - 1);
    return ret;
}

/**
 * A helper method that is called to process a given input file
 * with data in HTTP-GET format.  This method must outputs a system output of
 * the number of lines and hacks detected
 *
 * @param is The input stream of ssh logs
 */
void processUrl(std::string task, std::istream& is) {
    // Skipping to the bottom of the web-server
    ChildProcess cp;
    for (std::string hdr; std::getline(is, hdr) && !hdr.empty() && hdr != "\r";)
    {}
    for (std::string line; std::getline(is, line);) {
        if (line.substr(0, 1) == "#" || line.substr(0, 1) == "") {
            continue;
        } else {
            std::vector<std::string> vec;
            std::string val;
            std::istringstream is(line);
            while (is >> std::quoted(val)) {
                vec.push_back(val);
            }

            if (task == "SERIAL") {
                cout << "Running: " << putTogether(vec) << endl;
                cp.forkNexec(vec);
                cout << "Exit code: " << cp.wait() << endl;
            }
        }
    }
}

/**
 * A helper method that breaks down the url to match the format, calls the
 * setupDownload method and processUrl to store everything in a vector.
 * 
 * @param task The task we want to execute, serial or parallel
 * 
 * @param url The url we want to breakdown to match the format
 */
void readUrl(std::string task, std::string url) {
    std::vector<std::string> vec;

    // Breaking down the url using two delimiters
    std::string delim1 = "//";
    std::string delim = "edu";
    std::string host = url.substr(url.find(delim1) + 2, url.find(delim) +
    delim.size() - url.find(delim1) - 2);
    std::string path = url.substr(url.find(delim) + delim.size(),
    url.size());

    tcp::iostream is;  // stream to read ssh logs
    setupDownload(host, path, is);  // calling this method to access the web url
    processUrl(task, is);
}

/**
 * A helper method stores a line of string into a vector word by word.
 * 
 * @param str The string we want to store in a vector
 *
 * @return A vector with all the broken down information in the url stored
 */
std::vector<std::string> stringToVec(std::string str) {
    std::istringstream is(str);
    string word;

    vector<string> v;

    while (is >> std::quoted(word)) {
        v.push_back(word);
    }

    return v;
}

/**
 * A helper method stores a line of string into a vector word by word.
 * 
 * @param s The string we want to store in a vector
 *
 * @return the formatted string
 */
std::string format(std::string s) {
    s.erase(remove(s.begin(), s.end(), '\"'), s.end());
    if (s.substr(0, 1) == " ") {
        return s.substr(1);
    } else {
        return s;
    }
}

/**
 * A top level method that takes the user input and executes them on the
 * terminal. The user can either call a command line arguement or call
 * series or parallel on a web url.
 * 
 * @param is the user input
 * 
 * @param prompt the carrot to prompt user input
 */
void process(std::istream& is = std::cin, const std::string& prompt = "> ") { 
    // Adapt the following loop as you see fit
    std::string line;
    ChildProcess cp;
    while (std::cout << prompt, std::getline(std::cin, line)) {
        // Process the input line here.
        std::string firstW;
        std::istringstream is(line);
        is >> firstW;

        // Checking for comments
        if (firstW != "#") {
            if (firstW == "SERIAL" || firstW == "PARALLEL") {
                std::string task = firstW;
                is >> firstW;
                readUrl(task, firstW);
            } else {
                if (firstW == "") {
                    continue;
                } else if (firstW == "exit") { break; } else {
                    cout << "Running: " << format(line) << endl;
                    cp.forkNexec(stringToVec(line));
                    cout << "Exit code: " << cp.wait() << endl;
                }
            }
        }
    }
}

/**
 * The main method that takes a user input and calls the process method.
 */
int main(int argc, char *argv[]) {
    // std::string input;
    process(std::cin);

    return 0;
}

// End of source code
