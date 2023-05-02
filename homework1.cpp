// Copyright 2023 Brendan Han
// CSE381
// Homework01

/**
 * A simple program that:
 *    1. processes input in the form of HTTP-GET or HTTP-POST request
 *    2. Uses input to process an input file (in HTTP response format)
 *    3. Generates output in HTTP response format.
 */
#include <bits/stdc++.h>
#include <boost/format.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <iterator>

std::string url_decode(std::string str);
std::unordered_map<std::string, std::string> processParams(std::string params);
int minVal(std::istream& is);
int max(std::istream& is);

/** A convenience format string to generate results in HTML
 *   format. Note that this format string has place holders in the form
 *   %1%, %2% etc.  These are filled-in with actual values.  For
 *   example, you can generate actual values as shown below:
 *
 *  \code
 *
 *   auto html = boost::str(boost::format(ResultData) %
 *                          inputFile % function % dataType % result);
 *
 *  \endcode
*/
const std::string ResultData = R"(<html>
  <body>
    <h2>Analysis results</h2>
    <p>Input data file path: %1%</p>
    <p>Function requested: %2%</p>
    <p>Data type of input data: %3%</p>
    <p>Result: %4%</p>
  </body>
</html>
)";

/**
 * A convenience format string to generate HTTP-response based on the
 * length (aka size) of the response (typically an HTML data).  Note
 * that this format string has place holders in the form %1%, %2% etc.
 * These are filled-in with actual values.  For example, you can
 * generate actual values as shown below:
 *
 *   \code
 *
 *   const std::string data = "some data";
 *   
 *   auto hdr = boost::str(boost::format(HTMLRespHeader) % data.size());
 *
 *   \endcode
 */
const std::string HTTPRespHeader = "HTTP/1.1 200 OK\r\n"
    "Server: SimpleServer\r\n"
    "Content-Length: %1%\r\n"
    "Connection: Close\r\n"
    "Content-Type: text/html\r\n\r\n";

std::string url_decode(std::string str) {
  size_t pos = 0;
  while ((pos = str.find_first_of("%+", pos)) != std::string::npos) {
    switch (str.at(pos)) {
      case '+': str.replace(pos, 1, " ");
      break;
      case '%': {
        std::string hex = str.substr(pos + 1, 2);
        char ascii = std::stoi(hex, nullptr, 16);
        str.replace(pos, 3, 1, ascii);
      }  // decode '%xx'
    }
    pos++;
  }
  return str;
}

/**
 * A helper method that change the delimiters '=' and '&' with blank spaces to
 * ease using a string stream to read name -- value pairs. Also reads name value
 * pairs and adds them to an unordered map to ease further processing
 *
 * @param params The string line in the text file to be read.
 */
std::unordered_map<std::string, std::string> processParams(std::string params) {
  std::replace(params.begin(), params.end(), '=', ' ');
  std::replace(params.begin(), params.end(), '&', ' ');
  std::unordered_map<std::string, std::string> nameVal;
  std::istringstream is(params);

  for (std::string name, val; is >> name >> val;) {
    nameVal[name] = url_decode(val);
  }
  // Return the map of parameters to ease processing them.
  return nameVal;
}

/**
 * A helper method that computes the max value in a text file
 *
 * @param is The input stream from where the input data file is to be
 * read.
 */
int max(std::string f) {
  std::ifstream file(f);
  std::istream& in(file);  // Streaming file

  // Going to the end of the file
  for (std::string hdr; std::getline(in, hdr) && !hdr.empty() && hdr != "\r";)
  {}
  int larVal = INT_MIN, val = 0;
  in >> val;

  while (in >> val) {
      if (val > larVal) {
          larVal = val;
      }
  }

  return larVal;
}

/**
 * A helper method that computes the second min value in a text file
 *
 * @param is The input stream from where the input data file is to be
 * read.
 */
int minVal(std::string f) {
  std::ifstream file(f);
  std::istream& in(file);  // Streaming file

  // Going to the end of the file
  for (std::string hdr; std::getline(in, hdr) && !hdr.empty() && hdr != "\r";)
  {}
  int larVal = 0, secLarVal = 0, val = 0;
  in >> larVal >> secLarVal;

  if (larVal > secLarVal) {
      std::swap(larVal, secLarVal);
  }
  while (in >> val) {
      if (val < larVal) {
          secLarVal = larVal;
          larVal = val;
      } else if (val < secLarVal) {
          secLarVal = val;
      }
  }

  return secLarVal;
}

/**
 * The top-level method that is called to process a given input file
 * with data in either HTTP-GET or HTTP-POST format.  This method must
 * generate output in an HTTP-response format using the format strings
 * ResultData and HTTPRespHeader.
 *
 * @param is The input stream from where the input data file is to be
 * read.
 *
 * @param os The output stream to where the results are to be
 * printed. Note: Do not print to std::cout. Instead, print to this
 * output stream.
 */
void process(std::istream& is, std::ostream& os) {
  // Suitably implement this method using helper methods to
  // structure your logic. Add helper methods *before* this method.
  std::unordered_map<std::string, std::string> map;
  std::string input;
  is >> input;
  std::string line;
  std::getline(is, line);
  
  if (input == "POST") {
    for (std::string hdr; std::getline(is, hdr) && !hdr.empty() && hdr != "\r";)
    {}
    std::string str;
    std::getline(is, str);
    map = processParams(str);
  } else if (input == "GET") {
    line = line.substr(line.find('?') + 1, line.length());
    map = processParams(line);
  }

  // Generating results to be sent back to the client in HTML format.
  auto htmlData = boost::str(boost::format(ResultData) 
  % map["file"] 
  % map["func"]
  % map["type"]
  % (map["func"] == "min2nd" ? minVal(map["file"]) : max(map["file"])));
  
  // Now that we have HTML data, we can fill-in the content length
  // value in the HTTP response header.
  auto httpRespHdr =
    boost::str(boost::format(HTTPRespHeader) % htmlData.size());

  // Generate the response in HTTP-response format.
  os << httpRespHdr << htmlData;
}

// End of source code
