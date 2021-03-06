#ifndef MAKESTRING_H_INCLUDED
#define MAKESTRING_H_INCLUDED


#include <sstream>
#include <string>


namespace Futile {


/**
 * MakeString
 *
 * Enables logging like this:
 *
 *   LogError(MakeString() << "The index " << i << " exceeds the max length of " << max << ".");
 *
 */
class MakeString
{
public:
    template <typename T>
    MakeString& operator<<(const T& datum)
    {
        mBuffer << datum;
        return *this;
    }
    operator std::string() const
    {
        return mBuffer.str();
    }
private:
    std::ostringstream mBuffer;
};


typedef MakeString Str;


} // namespace Futile


#endif // MAKESTRING_H_INCLUDED
