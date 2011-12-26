#ifndef STMSUPPORT_H
#define STMSUPPORT_H


#include <functional>
#include "stm.hpp"


namespace Futile {
namespace STM {



template<class T>
inline T get(stm::shared<T> & inSharedValue)
{
    return stm::atomic<T>([&](stm::transaction & tx) {
        return inSharedValue.open_r(tx);
    });
}


template<class T>
inline void set(stm::shared<T> & outSharedValue, const T & inValue)
{
    return stm::atomic([&](stm::transaction & tx) {
        T & value = outSharedValue.open_rw(tx);
        value = inValue;
    });
}


#if 0
template<class T>
inline void read(const stm::shared<T> & inSharedValue, std::function<void(const T&)> inFunctor)
{
    stm::atomic([&](stm::transaction & tx) {
        const T & value = inSharedValue.open_r(tx);
        inFunctor(value);
    });
}


template<class T>
inline void write(stm::shared<T> & inSharedValue, std::function<void(T&)> inFunctor)
{
    stm::atomic([&](stm::transaction & tx) {
        T & value = inSharedValue.open_rw(tx);
        inFunctor(value);
    });
}
#endif


} } // namespace Futile::STM


#endif // STMSUPPORT_H
