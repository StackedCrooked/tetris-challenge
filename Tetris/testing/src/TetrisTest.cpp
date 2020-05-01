#include "TetrisTest.h"
#include "Futile/Threading.h"
#include <cstdlib>


namespace testing {


std::string GetEnv(const std::string& name, const std::string& def) {
    const char * res = getenv(name.c_str());
    return res ? res : def;
}


void TetrisTest::BeBusy()
{
    while (true)
    {
        bool interruptRequest = boost::this_thread::interruption_requested();
        boost::this_thread::interruption_point();
        if (interruptRequest)
        {
            throw std::runtime_error("Interrupt requested!");
        }
        Futile::Sleep(100);
    }
}


} // namespace testing
