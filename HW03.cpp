// Copyright Brendan Han 2023

/**
 * A program to detect potential attempts at trying to break into
 * accounts by scanning logs on a Linux machine. Breakin attempts are
 * detected using the two rules listed further below.
 *
 *   1. If an IP is in the "banned list", then it is flagged as a
 *      break in attempt.
 *
 *   2. unless an user is in the "authorized list", if an user has
 *      attempted to login more than 3 times in a span of 20 seconds
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <boost/asio.hpp>

// Convenience namespace declarations to streamline the code below
using namespace boost::asio;
using namespace boost::asio::ip;
using namespace std;

/** Synonym for an unordered map that is used to track banned IPs and 
 * authorized users. For example, the key in this map would be IP addresses
 * and the value is just a place holder (is always set to true).
 */
using LookupMap = std::unordered_map<std::string, bool>;

/**
 * An unordered map to track the seconds for each log entry associated
 * with each user. The user ID is the key into this unordered map.
 * The values is a list of timestamps of log entries associated with
 * an user. For example, if a user "bob" has 3 login at "Aug 29 11:01:01",
 * "Aug 29 11:01:02", and "Aug 29 11:01:03" (one second apart each), then
 * logins["bill"] will be a vector with values {1630249261, 1630249262, 
 * 1630249263}. 
 */
using LoginTimes = std::unordered_map<std::string, std::vector<long>>;

/**
 * Helper method to load data from a given file into an unordered map.
 * 
 * @param fileName The file name from words are are to be read by this 
 * method. The parameter value is typically "authorized_users.txt" or
 * "banned_ips.txt".
 * 
 * @return Return an unordered map with the 
 */
LookupMap loadLookup(const std::string& fileName) {
    // Open the file and check to ensure that the stream is valid
    std::ifstream is(fileName);
    if (!is.good()) {
        throw std::runtime_error("Error opening file " + fileName);
    }
    // The look up map to be populated by this method.
    LookupMap lookup;
    // Load the entries into the unordered map
    for (std::string entry; is >> entry;) {
        lookup[entry] = true;
    }
    // Return the loaded unordered map back to the caller.
    return lookup;
}

/**
 * This method is used to convert a timestamp of the form "Jun 10
 * 03:32:36" to seconds since Epoch (i.e., 1900-01-01 00:00:00). This
 * method assumes by default, the year is 2021.
 *
 * \param[in] timestamp The timestamp to be converted to seconds.  The
 * timestamp must be in the format "Month day Hour:Minutes:Seconds",
 * e.g. "Jun 10 03:32:36".
 *
 * \param[in] year An optional year associated with the date. By
 * default this value is assumed to be 2021.
 *
 * \return This method returns the seconds elapsed since Epoch.
 */
long toSeconds(const std::string& timestamp, const int year = 2021) {
    // Initialize the time structure with specified year.
    struct tm tstamp = { .tm_year = year - 1900 };
    // Now parse out the values from the supplied timestamp
    strptime(timestamp.c_str(), "%B %d %H:%M:%S", &tstamp);
    // Use helper method to return seconds since Epoch
    return mktime(&tstamp);
}

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
 * The top-level method that is called to process a given input file
 * with data in HTTP-GET format.  This method must outputs a system output of
 * the number of lines and hacks detected
 *
 * @param is The input stream of ssh logs
 *
 * @param os The output stream to where the results are to be
 * printed.
 */
void process(std::istream& is, std::ostream& os) {
    LookupMap goodUsers = loadLookup("authorized_users.txt");
    LookupMap bannedIps = loadLookup("banned_ips.txt");
    int lineCount = 0, hackCount = 0, failCount = 0;

    // Skipping to the bottom of the web-server
    for (std::string hdr; std::getline(is, hdr) && !hdr.empty() && hdr != "\r";)
    {}
    for (std::string line; std::getline(is, line);) {
        // Instead of printing lines, do the necessary processing to
        // detect malicious logins
        lineCount++;

        // Reading through each criteria of the line
        std::string month, day, time, user, ip, status;
        std::istringstream(line) >> month >> day >> time >> status >> status >>
        status >> user >> user >> user >> ip >> ip;
        std::string tempStatus = status;

        // Counting for repeated failed login attempts
        failCount = (status == "Failed" && status == tempStatus) ?
        failCount + 1 : 0;

        if (failCount >= 3) {
            hackCount++;
            std::cout << "Hacking due to frequency. Line: " << line << '\n';
        }

        // Checking in the unordered map if the current ip is a banned ip
        if (bannedIps[ip]) {
            hackCount++;
            std::cout << "Hacking due to banned IP. Line: " << line << '\n';
        }
    }
    std::cout << "Processed " << lineCount << " lines. Found " << hackCount
    << " possible hacking attempts.\n";
}

/**
 * The main function that uses different helper methods to download and process
 * log entries from the given URL and detect potential hacking attempts.
 *
 * \param[in] argc The number of command-line arguments.  This program
 * requires exactly one command-line argument.
 *
 * \param[in] argv The actual command-line argument. This should be an URL.
 */
int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cout << "Specify URL from where logs are to be obtained.\n";
        return 1;  // non-zero return to indicate error.
    }
    // Store the URL as a string to make processing easier.
    const std::string url = argv[1];
    std::string delim1 = "//";
    std::string delim = "edu";
    std::string host = url.substr(url.find(delim1) + 2, url.find(delim) +
    delim.size() - url.find(delim1) - 2);
    std::string path = url.substr(url.find(delim) + delim.size(),
    url.size());
    // Using helper methods, implement the neccessary features for
    // this project.
    // To help you get started, here is an hard coded example that
    // simply prints the response from a web-server.
    tcp::iostream is;  // stream to read ssh logs
    setupDownload(host, path, is);

    // Calling the process method to output the results from the web-server
    process(is, cout);

    // All done. Successful finish should return zero.
    return 0;
}

// End of source code
