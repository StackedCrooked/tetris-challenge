//
// PartHandler.h
//
// $Id: //poco/1.3/Net/include/Poco/Net/PartHandler.h#1 $
//
// Library: Net
// Package: Messages
// Module:  PartHandler
//
// Definition of the PartHandler class.
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


#ifndef Net_PartHandler_INCLUDED
#define Net_PartHandler_INCLUDED


#include "Poco/Net/Net.h"
#include <istream>


namespace Poco {
namespace Net {


class MessageHeader;


class Net_API PartHandler
	/// The base class for all part or attachment handlers.
	///
	/// Part handlers are used for handling email parts and 
	/// attachments in MIME multipart messages, as well as file 
	/// uploads via HTML forms.
	///
	/// Subclasses must override handlePart().
{
public:
	virtual void handlePart(const MessageHeader& header, std::istream& stream) = 0;
		/// Called for every part encountered during the processing
		/// of an email message or an uploaded HTML form.
		///
		/// Information about the part can be extracted from
		/// the given message header. What information can be obtained
		/// from header depends on the kind of part.
		///
		/// The content of the part can be read from stream.
		
protected:
	PartHandler();
		/// Creates the PartHandler.

	virtual ~PartHandler();
		/// Destroys the PartHandler.

private:
	PartHandler(const PartHandler&);
	PartHandler& operator = (const PartHandler&);
};


} } // namespace Poco::Net


#endif // Net_PartHandler_INCLUDED
