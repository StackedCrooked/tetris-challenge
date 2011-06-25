//
// QuotedPrintableEncoder.h
//
// $Id: //poco/1.3/Net/include/Poco/Net/QuotedPrintableEncoder.h#1 $
//
// Library: Net
// Package: Messages
// Module:  QuotedPrintableEncoder
//
// Definition of the QuotedPrintableEncoder class.
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


#ifndef Net_QuotedPrintableEncoder_INCLUDED
#define Net_QuotedPrintableEncoder_INCLUDED


#include "Poco/Net/Net.h"
#include "Poco/UnbufferedStreamBuf.h"
#include <ostream>


namespace Poco {
namespace Net {


class Net_API QuotedPrintableEncoderBuf: public Poco::UnbufferedStreamBuf
	/// This streambuf encodes all data written
	/// to it in quoted-printable encoding (see RFC 2045)
	/// and forwards it to a connected ostream.
{
public:
	QuotedPrintableEncoderBuf(std::ostream& ostr);
	~QuotedPrintableEncoderBuf();
	int close();
	
private:
	int writeToDevice(char c);
	void writeEncoded(char c);
	void writeRaw(char c);

	int           _pending;
	int           _lineLength;
	std::ostream& _ostr;
};


class Net_API QuotedPrintableEncoderIOS: public virtual std::ios
	/// The base class for QuotedPrintableEncoder.
	///
	/// This class is needed to ensure the correct initialization
	/// order of the stream buffer and base classes.
{
public:
	QuotedPrintableEncoderIOS(std::ostream& ostr);
	~QuotedPrintableEncoderIOS();
	int close();
	QuotedPrintableEncoderBuf* rdbuf();

protected:
	QuotedPrintableEncoderBuf _buf;
};


class Net_API QuotedPrintableEncoder: public QuotedPrintableEncoderIOS, public std::ostream
	/// This ostream encodes all data
	/// written to it in quoted-printable encoding
	/// (see RFC 2045) and forwards it to a connected ostream.
	/// Always call close() when done
	/// writing data, to ensure proper
	/// completion of the encoding operation.
{
public:
	QuotedPrintableEncoder(std::ostream& ostr);
	~QuotedPrintableEncoder();
};


} } // namespace Poco::Net


#endif // Net_QuotedPrintableEncoder_INCLUDED
