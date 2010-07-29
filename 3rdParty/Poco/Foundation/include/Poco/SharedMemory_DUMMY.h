//
// SharedMemoryImpl.h
//
// $Id: //poco/1.3/Foundation/include/Poco/SharedMemory_DUMMY.h#2 $
//
// Library: Foundation
// Package: Processes
// Module:  SharedMemoryImpl
//
// Definition of the SharedMemoryImpl class.
//
// Copyright (c) 2007, Applied Informatics Software Engineering GmbH.
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


#ifndef Foundation_SharedMemoryImpl_INCLUDED
#define Foundation_SharedMemoryImpl_INCLUDED


#include "Poco/Foundation.h"
#include "Poco/SharedMemory.h"
#include "Poco/RefCountedObject.h"


namespace Poco {


class Foundation_API SharedMemoryImpl: public RefCountedObject
	/// A dummy implementation of shared memory, for systems
	/// that do not have shared memory support.
{
public:
	SharedMemoryImpl(const std::string& id, std::size_t size, SharedMemory::AccessMode mode, const void* addr, bool server);
		/// Creates or connects to a shared memory object with the given name.
		///
		/// For maximum portability, name should be a valid Unix filename and not
		/// contain any slashes or backslashes.
		///
		/// An address hint can be passed to the system, specifying the desired
		/// start address of the shared memory area. Whether the hint
		/// is actually honored is, however, up to the system. Windows platform
		/// will generally ignore the hint.

	SharedMemoryImpl(const Poco::File& aFile, SharedMemory::AccessMode mode, const void* addr);
		/// Maps the entire contents of file into a shared memory segment.
		///
		/// An address hint can be passed to the system, specifying the desired
		/// start address of the shared memory area. Whether the hint
		/// is actually honored is, however, up to the system. Windows platform
		/// will generally ignore the hint.

	char* begin() const;
		/// Returns the start address of the shared memory segment.

	char* end() const;
		/// Returns the one-past-end end address of the shared memory segment. 

protected:
	~SharedMemoryImpl();
		/// Destroys the SharedMemoryImpl.

private:
	SharedMemoryImpl();
	SharedMemoryImpl(const SharedMemoryImpl&);
	SharedMemoryImpl& operator = (const SharedMemoryImpl&);
};


//
// inlines
//
inline char* SharedMemoryImpl::begin() const
{
	return 0;
}


inline char* SharedMemoryImpl::end() const
{
	return 0;
}


} // namespace Poco


#endif // Foundation_SharedMemoryImpl_INCLUDED
