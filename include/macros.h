#ifndef MACROS_H
#define MACROS_H

[[noreturn]] void r_assert_failed(
    const char *__assertion, const char *__file, unsigned int __line, const char *__function);

#define r_assert(expr) \
    static_cast<bool>(expr) ? (void)0 : r_assert_failed(#expr, __FILE__, __LINE__, __func__)

#define PROJECT_NAME "de.molytho.launcher"

#endif