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


class Transaction
{
public:
    void operator()(stm::transaction & tx)
    {
        run(tx);
        for (auto fun : mFunctions) { fun(); }
    }

    typedef std::function<void(stm::transaction & tx)> TxFunction;

protected:
    typedef std::function<void()> Function;

    void invokeLater(const Function & inFunction)
    { mFunctions.push_back(inFunction); }

private:
    virtual void run(stm::transaction & ) = 0;

    typedef std::vector<Function> Functions;
    Functions mFunctions;
};


} } // namespace Futile::STM


#endif // STMSUPPORT_H
