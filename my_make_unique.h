#ifndef MY_MAKE_UNIQUE_H
#define MY_MAKE_UNIQUE_H

#include <memory>

template<class T, class... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

#endif // MY_MAKE_UNIQUE_H

