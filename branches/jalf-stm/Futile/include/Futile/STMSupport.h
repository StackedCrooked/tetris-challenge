#ifndef STMSUPPORT_H
#define STMSUPPORT_H


#include <functional>
#include "stm.hpp"


namespace Futile {
namespace STM {


template<typename T>
inline void set(stm::shared<T> & dst, const T & val)
{
    stm::atomic([&](stm::transaction & tx){ dst.open_rw(tx) = val; });
}


template<typename T>
inline T get(stm::shared<T> & src)
{
    return stm::atomic<T>([&](stm::transaction & tx){ return src.open_r(tx); });
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
