#ifndef INCLUDE_AL_GENERAL_UTILS_HPP
#define INCLUDE_AL_GENERAL_UTILS_HPP

#include <iostream>
#include <string>
#include <fstream>

std::string al_file_to_string(std::string path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        std::cout << "couldn't find " << path << std::endl;
        return "";
    }
    else return std::string {
        std::istreambuf_iterator<char> {f}, // from
        std::istreambuf_iterator<char> {} // to (0 argument returns end iterator)
    };
}

#endif