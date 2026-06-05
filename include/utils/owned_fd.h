#ifndef OWNED_FD_H
#define OWNED_FD_H

#include <errno.h>
#include <stdexcept>
#include <system_error>
#include <unistd.h>

class owned_fd {
    int m_fd;

public:
    constexpr explicit owned_fd(int fd = -1) : m_fd(fd) {}

    constexpr owned_fd(owned_fd &&other) : m_fd(other.m_fd) { other.m_fd = -1; }

    ~owned_fd() { reset(); }

    constexpr void reset(int fd = -1) {
        if (m_fd >= 0) {
            close(m_fd);
        }
        m_fd = fd;
    }

    constexpr operator int() const {
        if (!m_fd) {
            throw std::logic_error("invalid owned_fd accessed");
        }
        return m_fd;
    }

    constexpr explicit operator bool() const noexcept { return m_fd >= 0; }
};

inline owned_fd make_checked_owned_fd(int fd) {
    if (fd < 0) {
        throw std::system_error(errno, std::system_category());
    }
    return owned_fd(fd);
}

#endif