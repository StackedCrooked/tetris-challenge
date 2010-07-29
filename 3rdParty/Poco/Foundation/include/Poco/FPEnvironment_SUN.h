//
// FPEnvironment_SUN.h
//
// $Id: //poco/1.3/Foundation/include/Poco/FPEnvironment_SUN.h#1 $
//
// Library: Foundation
// Package: Core
// Module:  FPEnvironment
//
// Definitions of class FPEnvironmentImpl for Solaris.
//
// Copyright (c) 2005-2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
// 
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//


#ifndef Foundation_FPEnvironment_SUN_INCLUDED
#define Foundation_FPEnvironment_SUN_INCLUDED


#include "Poco/Foundation.h"
#include <ieeefp.h>


namespace Poco {


class FPEnvironmentImpl
{
protected:
	enum RoundingModeImpl
	{
		FP_ROUND_DOWNWARD_IMPL   = FP_RM,
		FP_ROUND_UPWARD_IMPL     = FP_RP,
		FP_ROUND_TONEAREST_IMPL  = FP_RN,
		FP_ROUND_TOWARDZERO_IMPL = FP_RZ
	};
	enum FlagImpl
	{
		FP_DIVIDE_BY_ZERO_IMPL = FP_X_DZ,
		FP_INEXACT_IMPL        = FP_X_IMP,
		FP_OVERFLOW_IMPL       = FP_X_OFL,
		FP_UNDERFLOW_IMPL      = FP_X_UFL,
		FP_INVALID_IMPL        = FP_X_INV
	};
	FPEnvironmentImpl();
	FPEnvironmentImpl(const FPEnvironmentImpl& env);
	~FPEnvironmentImpl();
	FPEnvironmentImpl& operator = (const FPEnvironmentImpl& env);
	void keepCurrentImpl();		
	static void clearFlagsImpl();
	static bool isFlagImpl(FlagImpl flag);	
	static void setRoundingModeImpl(RoundingModeImpl mode);
	static RoundingModeImpl getRoundingModeImpl();
	static bool isInfiniteImpl(float value);		
	static bool isInfiniteImpl(double value);
	static bool isInfiniteImpl(long double value);
	static bool isNaNImpl(float value);		
	static bool isNaNImpl(double value);
	static bool isNaNImpl(long double value);
	static float copySignImpl(float target, float source);		
	static double copySignImpl(double target, double source);
	static long double copySignImpl(long double target, long double source);

private:
	fp_rnd    _rnd;
	fp_except _exc;
};


} // namespace Poco


#endif // Foundation_FPEnvironment_SUN_INCLUDED
