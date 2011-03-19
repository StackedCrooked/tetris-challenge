#ifndef TETRIS_THREADINGCONFIGURATION_H
#define TETRIS_THREADINGCONFIGURATION_H


//
// Choose the preferred threading library here.
//
#define TETRIS_THREADING_LIBRARY_BOOST 1
#define TETRIS_THREADING_LIBRARY_POCO  0


//
// Load the configuration settings.
//
#if TETRIS_THREADING_LIBRARY_BOOST
#include "Futile/ThreadingConfigurationBoost.h"
#elif TETRIS_THREADING_LIBRARY_POCO
#include "Futile/ThreadingConfigurationPoco.h"
#endif


#endif // TETRIS_THREADINGCONFIGURATION_H
