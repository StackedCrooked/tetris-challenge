#ifndef ERROR_HANDLING_INCLUDED
#define ERROR_HANDLING_INCLUDED


#include <stdexcept>
#include <string>


namespace Tetris
{
    
   inline void LogicAssert(bool inCondition, const std::string & inErrorMessage)
   {
       if (!inCondition)
       {
           throw std::logic_error(inErrorMessage);
       }
   }
    
   inline void RuntimeAssert(bool inCondition, const std::string & inErrorMessage)
   {
       if (!inCondition)
       {
           throw std::runtime_error(inErrorMessage);
       }
   }

} // namespace Tetris


#endif // ERROR_HANDLING_INCLUDED
