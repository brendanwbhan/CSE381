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

// It is ok to use the following namespace delarations in C++ source
// files only. They must never be used in header files.
using namespace std;
using namespace std::string_literals;

std::unordered_map<std::string, std::string> storeUid(std::string f);
std::unordered_map<std::string, std::string> storeGid(std::string f);
std::unordered_map<std::string, std::string> storeGroups(std::string f);
void process(std::istream& is, std::ostream& os);

/**
 * A helper method that store information from passwd file into an unordered map
 * Example: [0] = "root"
 * 
 * @param params The string f, the text file to be read.
 */
std::unordered_map<std::string, std::string> storeUid(std::string f) {
    std::unordered_map<std::string, std::string> userInfo;
    std::ifstream file(f);
    std::string line;

    // Reading line-by-line
    while (std::getline(file, line)) {
        std::replace(line.begin(), line.end(), ':', ' ');
        std::istringstream is(line);

        // Storing values in the unordered map
        for (std::string loginId, uid; is >> loginId >> uid >> uid;) {
            userInfo[uid] = loginId;
            break;
        }
    }
    // Return the map of parameters to ease processing them.
    return userInfo;
}

/**
 * A helper method that store information from groups file into an unordered map
 * Example: [2] = 1000,1001,1003,1004,2000,1500,2001,2002,2010,2011,2012
 * 
 * @param params The string f, the text file to be read.
 */
std::unordered_map<std::string, std::string> storeGid(std::string f) {
    std::unordered_map<std::string, std::string> groupInfo;
    std::ifstream file(f);
    std::string line;

    // Reading line-by-line
    while (std::getline(file, line)) {
        std::replace(line.begin(), line.end(), ':', ' ');
        std::istringstream is(line);

        // Storing values in the unordered map
        for (std::string group, gid; is >> gid >> gid >> gid >> group;) {
            if (group != "")
                groupInfo[gid] = group;
            else 
                groupInfo[gid] = " ";
            break;
        }
    }
    // Return the map of parameters to ease processing them.
    return groupInfo;
}

/**
 * A helper method that store information from groups file into an unordered map
 * Example: [root] = 0
 * 
 * @param params The string f, the text file to be read.
 */
std::unordered_map<std::string, std::string> storeGroups(std::string f) {
    std::unordered_map<std::string, std::string> groups;
    std::ifstream file(f);
    std::string line;

    // Reading line-by-line
    while (std::getline(file, line)) {
        std::replace(line.begin(), line.end(), ':', ' ');
        std::istringstream is(line);

        // Storing values in the unordered map
        for (std::string group, gid; is >> group >> gid >> gid;) {
            if (group != "")
                groups[gid] = group;
            else 
                groups[gid] = " ";

            break;
        }
    }
    // Return the map of parameters to ease processing them.
    return groups;
}

/**
 * A helper method that stores information all the necessary information
 * into an unordered map. Then forms an output using those information.
 * 
 * @param params The string in, the command line argument.
 */
std::string result(std::string in) {
    std::unordered_map<std::string, std::string> map1;
    std::unordered_map<std::string, std::string> map2;
    std::unordered_map<std::string, std::string> map3;
    std::string ret;
    // Storing unorder map with necessary textfile information
    map1 = storeUid("passwd");
    map2 = storeGid("groups");
    map3 = storeGroups("groups");

    // checking if the input key exists
    if (map3.find(in) == map3.end()) {
        return in + " = Group not found.";
    } else {
        try {
            ret = in + " = " + map3[in] + ":";
            
            // Taking in all the user associated with the groupId
            string users = map2[in];
            std::replace(users.begin(), users.end(), ',', ' ');
            istringstream iss(users);

            // Reading string word-by-word to identify group members associated
            // with the uid
            string sub;
            while (iss >> sub) {
                ret += " ";
                ret += (map1[sub] + "(" + sub + ")");
            }
        } catch (...) {}
    }
    return ret;
}

/**
 * The main function that takes the input command and calls the result method to
 * formulate an output.
 *
 * \param[in] argc The number of command-line arguments. This value is
 * expected to be 2 as this program expects exactly 1 command-line argument.
 *
 * \param[in] argv The actual command-line arguments. The 1st command-line
 * argument is assumed to be the path to the input file to be processed by
 * this program.
 */
int main(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        cout << result(argv[i]);
        cout << "\n";
    }

    // All done.
    return 0;
}

// End of source code
