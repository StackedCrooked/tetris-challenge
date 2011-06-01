//
// BasicEvent.h
//
// $Id: //poco/1.3/Foundation/include/Poco/BasicEvent.h#2 $
//
// Library: Foundation
// Package: Events
// Module:  BasicEvent
//
// Implementation of the BasicEvent template.
//
// Copyright (c) 2006, Applied Informatics Software Engineering GmbH.
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


#ifndef  Foundation_BasicEvent_INCLUDED
#define  Foundation_BasicEvent_INCLUDED


#include "Poco/AbstractEvent.h"
#include "Poco/DefaultStrategy.h"
#include "Poco/AbstractDelegate.h"
#include "Poco/CompareFunctions.h"


namespace Poco {


template <class TArgs> 
class BasicEvent: public AbstractEvent < 
	TArgs, DefaultStrategy<TArgs, AbstractDelegate<TArgs>, p_less<AbstractDelegate<TArgs> > >,
	AbstractDelegate<TArgs> 
>
	/// A BasicEvent uses internally a DefaultStrategy which 
	/// invokes delegates in an arbitrary manner.
	/// Note that one object can only register one method to a BasicEvent.
	/// Subsequent registrations will overwrite the existing delegate.
	/// For example:
	///     BasicEvent<int> event;
	///     MyClass myObject;
	///     event += delegate(&myObject, &MyClass::myMethod1);
	///     event += delegate(&myObject, &MyClass::myMethod2);
	///
	/// The second registration will overwrite the first one. The reason is simply that
	/// function pointers can only be compared by equality but not by lower than.
{
public:
	BasicEvent()
	{
	}

	~BasicEvent()
	{
	}

private:
	BasicEvent(const BasicEvent& e);
	BasicEvent& operator = (const BasicEvent& e);
};


} // namespace Poco


#endif
