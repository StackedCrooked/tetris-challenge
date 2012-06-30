include_directories(
    ${SOLUTIONDIR}Futile/include
    ${SOLUTIONDIR}3rdParty/Boost)

set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX "-isystem ")

include_directories(SYSTEM
    ${SOLUTIONDIR}3rdParty/Boost
    ${SOLUTIONDIR}3rdParty/stm)

add_library(Futile
    include/Futile/Array.h
    include/Futile/Assert.h
    include/Futile/AutoPtrSupport.h
    include/Futile/Boost.h
    include/Futile/GenericGrid.h
    include/Futile/LeakDetector.h
    include/Futile/Logger.h
    include/Futile/Logging.h
    include/Futile/MainThread.h
    include/Futile/MainThreadImpl.h
    include/Futile/MakeString.h
    include/Futile/PrettyPrint.h
    include/Futile/Stopwatch.h
    include/Futile/Threading.h
    include/Futile/Timer.h
    include/Futile/TypedWrapper.h
    include/Futile/Worker.h
    include/Futile/WorkerPool.h
    src/LeakDetector.cpp
    src/Logger.cpp
    src/Logging.cpp
    src/MainThread.cpp
    src/PrettyPrint.cpp
    src/Stopwatch.cpp
    src/Threading.cpp
    src/Timer.cpp
    src/Worker.cpp
    src/WorkerPool.cpp)

set_target_properties(Futile PROPERTIES
    COMPILE_FLAGS "${FINAL_COMPILE_FLAGS}")
