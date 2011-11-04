#include "TetrisTest.h"
#include "Futile/Threading.h"


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
