find_package(Boost)
find_package(Poco)

add_library(Futile
    include/Futile/Allocator.h
    include/Futile/Array.h
    include/Futile/Assert.h
    include/Futile/AutoPtrSupport.h
    include/Futile/Boost.h
    include/Futile/Config.h
    include/Futile/Enum.h
    include/Futile/GenericGrid.h
    include/Futile/LeakDetector.h
    include/Futile/Logger.h
    include/Futile/Logging.h
    include/Futile/MainThread.h
    include/Futile/MainThreadImpl.h
    include/Futile/MakeString.h
    include/Futile/MemoryPool.h
    include/Futile/Node.h
    include/Futile/Singleton.h
    include/Futile/Threading.h
    include/Futile/TypedWrapper.h
    include/Futile/Worker.h
    include/Futile/WorkerPool.h
    src/LeakDetector.cpp
    src/Logger.cpp
    src/Logging.cpp
    src/MainThread.cpp
    src/Threading.cpp
    src/Worker.cpp
    src/WorkerPool.cpp
)

target_include_directories(Futile PUBLIC include PRIVATE src)
target_link_libraries(Futile PUBLIC Poco::Poco Boost::thread)
