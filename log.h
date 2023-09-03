#pragma once

#include <string>

namespace Log {
    template<typename T>
    T LogError(const char * Str) {
        fprintf(stderr, "Error: %s\n", Str);
        return nullptr;
    }
};
