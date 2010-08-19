#ifndef ERROR_HANDLING_INCLUDED
#define ERROR_HANDLING_INCLUDED


#include <stdexcept>
#include <string>


namespace Tetris
{

    #define CheckCondition(...)
    #define CheckArgument(...)
    #define CheckPrecondition(...)

    //inline void CheckCondition(bool inCondition, const std::string & inErrorMessage)
    //{
    //    if (!inCondition)
    //    {
    //        throw std::logic_error(inErrorMessage);
    //    }
    //}

    //inline void CheckArgument(bool inCondition, const std::string & inErrorMessage)
    //{
    //    if (!inCondition)
    //    {
    //        throw std::invalid_argument(inErrorMessage);
    //    }
    //}

    //inline void CheckPrecondition(bool inCondition, const std::string & inErrorMessage)
    //{
    //    if (!inCondition)
    //    {
    //        throw std::logic_error(inErrorMessage);
    //    }
    //}

} // namespace Tetris


#endif // ERROR_HANDLING_INCLUDED
