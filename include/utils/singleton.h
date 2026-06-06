#ifndef SINGLETON_H
#define SINGLETON_H

#include <cassert>
#include <memory>

template<class T>
struct singleton_impl {
    static inline std::unique_ptr<T> m_instance {};

    template<class... Args>
    static void init(Args &&...args) {
        assert(!m_instance);
        m_instance = std::make_unique<T>(std::forward<Args>(args)...);
    }

    static T &get_instance() {
        assert(m_instance);
        return *m_instance;
    }
};

#endif