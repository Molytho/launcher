#ifndef MACROS_H
#define MACROS_H

#include <cstdlib>

// TODO: This should actually print some info
#define r_assert(cond) \
    if (!(cond)) {           \
        std::abort();        \
    }

#endif