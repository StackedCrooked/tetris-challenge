#ifndef FUTILE_THREADINGCONFIGURATION_H
#define FUTILE_THREADINGCONFIGURATION_H


//
// Choose the preferred threading library here.
//
#define FUTILE_THREADING_LIBRARY_BOOST 1
#define FUTILE_THREADING_LIBRARY_POCO  0


//
// Load the configuration settings.
//
#if FUTILE_THREADING_LIBRARY_BOOST
#include "Futile/ThreadingConfigurationBoost.h"
#elif FUTILE_THREADING_LIBRARY_POCO
#include "Futile/ThreadingConfigurationPoco.h"
#endif


#endif // FUTILE_THREADINGCONFIGURATION_H
