#include "utilities.h"


namespace Utilities {

    std::string read_file(const std::string &filePath)
    {
        std::ifstream file(filePath); // File is automatically closed by ifstream's destructor
        if (!file.is_open())
            throw std::runtime_error("Unable to open file: " + filePath);

        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

}