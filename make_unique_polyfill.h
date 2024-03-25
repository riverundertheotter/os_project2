#ifndef MAKE_UNIQUE_POLYFILL_H
#define MAKE_UNIQUE_POLYFILL_H

#include <memory>

template<class T, class... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

#endif // MAKE_UNIQUE_POLYFILL_H

