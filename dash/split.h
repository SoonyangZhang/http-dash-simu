#pragma once
//functions to split a string by a specific delimiter
#include <string>
#include <vector>
#include <sstream>
#include <string.h>
//https://github.com/ekg/split
// thanks to Evan Teran, http://stackoverflow.com/questions/236129/how-to-split-a-string/236803#236803
namespace basic{
// split a string on a single delimiter character (delim)
std::vector<std::string>& split(const std::string &s, char delim, std::vector<std::string> &elems);
std::vector<std::string>  split(const std::string &s, char delim);

// split a string on any character found in the string of delimiters (delims)
std::vector<std::string>& split(const std::string &s, const std::string& delims, std::vector<std::string> &elems);
std::vector<std::string>  split(const std::string &s, const std::string& delims);    
}
