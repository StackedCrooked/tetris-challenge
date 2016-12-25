#include "Futile/Threading.h"
#include <ctime>
#include <chrono>
#include <thread>


namespace Futile {


void Sleep(UInt64 inMilliseconds)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(inMilliseconds));
}


UInt64 GetCurrentTimeMs()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}


} // namespace Futile
