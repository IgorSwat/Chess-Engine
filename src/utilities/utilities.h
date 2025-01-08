#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <exception>


namespace Utilities {
    
    // Loads file to memory and converts to string representation
    std::string read_file(const std::string &filePath);
    
}