set(SolutionDir ..)
include_directories(
    include
    ${SolutionDir}/3rdParty/Boost
    ${SolutionDir}/3rdParty/Poco/Foundation/include
)


add_library(Futile
    include/Futile/Allocator.h
    include/Futile/Array.h
    include/Futile/Assert.h
    include/Futile/AutoPtrSupport.h
    include/Futile/Boost.h
    include/Futile/Config.h
    include/Futile/GenericGrid.h
    include/Futile/Grid.h
    include/Futile/Logger.h
    include/Futile/Logging.h
    include/Futile/MainThread.h
    include/Futile/MakeString.h
    include/Futile/Threading.h
    include/Futile/TypedWrapper.h
    include/Futile/Worker.h
    include/Futile/WorkerPool.h
    src/Logger.cpp
    src/Logging.cpp
    src/MainThread.cpp
    src/Threading.cpp
    src/Worker.cpp
    src/WorkerPool.cpp)

