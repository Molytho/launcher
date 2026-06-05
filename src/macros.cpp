#include <cstdlib>
#include <iostream>

[[noreturn]] void r_assert_failed(
    const char *__assertion, const char *__file, unsigned int __line, const char *__function) {
    std::cerr << "runtime assertion (" << __assertion << ") failed\n   in " << __function << " at " << __file << ':' << __line << '\n';
    std::abort();
}