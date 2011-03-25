#include "Futile/LeakDetector.h"
#include <iostream>


namespace Futile {


LeakDetector::LeakDetector()
{
}


LeakDetector::~LeakDetector()
{
    if (!mTypeInfos.empty())
    {
        std::cerr << "*** Leaks detected! ***" << std::endl;
        for (TypeInfos::const_iterator it = mTypeInfos.begin(); it != mTypeInfos.end(); ++it)
        {
            const std::type_info & typeInfo = *it->second;
            std::cerr << typeInfo.name() << std::endl;
        }
        std::cerr << std::flush;
    }
}


} // namespace Futile
