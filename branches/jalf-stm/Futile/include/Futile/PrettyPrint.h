#ifndef FUTILE_PRETTYPRINT_H
#define FUTILE_PRETTYPRINT_H


#include <ostream>
#include <vector>


namespace Futile {


template<class T>
inline std::ostream & operator<<(std::ostream & os, const std::vector<T> & vec)
{
    os << "[";
    for (typename std::vector<T>::size_type idx = 0; idx < vec.size(); ++idx)
    {
        if (idx != 0)
        {
            os << ", ";
        }
        os << vec[idx];
    }
    os << "]";
    return os;
}


} // namespace Futile


#endif // PRETTYPRINT_H
