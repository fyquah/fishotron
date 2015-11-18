//
// Created by Fu Yong Quah on 11/18/15.
//

#ifndef FISHOTRON_ATOMIC_H
#define FISHOTRON_ATOMIC_H

#include <mutex>

namespace fishotron {

template<typename T>
class Atomic {
private:
    std::mutex mtx;
    T val;
public:
    T load() {
        T tmp;
        mtx.lock();
        tmp = val;
        mtx.unlock();

        return tmp;
    }

    void store(T arg) {
        mtx.lock();
        val = arg;
        mtx.unlock();
    }

};

}

#endif //FISHOTRON_ATOMIC_H
