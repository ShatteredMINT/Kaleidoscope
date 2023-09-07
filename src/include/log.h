#pragma once

#include <string>

/**everything related to error logging*/
namespace Log {

    /**print error message and return nullptr of provided type*/
    template<typename T>
    T LogError(const char * Str) {
        fprintf(stderr, "Error: %s\n", Str);
        return nullptr;
    }
};
