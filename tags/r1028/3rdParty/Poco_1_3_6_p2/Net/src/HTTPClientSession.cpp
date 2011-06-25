//
// HTTPClientSession.cpp
//
// $Id: //poco/1.3/Net/src/HTTPClientSession.cpp#8 $
//
// Library: Net
// Package: HTTPClient
// Module:  HTTPClientSession
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


#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/HTTPHeaderStream.h"
#include "Poco/Net/HTTPStream.h"
#include "Poco/Net/HTTPFixedLengthStream.h"
#include "Poco/Net/HTTPChunkedStream.h"
#include "Poco/Net/NetException.h"
#include "Poco/NumberFormatter.h"
#include "Poco/CountingStream.h"


using Poco::NumberFormatter;
using Poco::IllegalStateException;


namespace Poco {
namespace Net {


HTTPClientSession::HTTPClientSession():
	_port(HTTPSession::HTTP_PORT),
	_proxyPort(HTTPSession::HTTP_PORT),
	_keepAliveTimeout(DEFAULT_KEEP_ALIVE_TIMEOUT, 0),
	_reconnect(false),
	_mustReconnect(false),
	_expectResponseBody(false),
	_pRequestStream(0),
	_pResponseStream(0)
{
}


HTTPClientSession::HTTPClientSession(const StreamSocket& socket):
	HTTPSession(socket),
	_port(HTTPSession::HTTP_PORT),
	_proxyPort(HTTPSession::HTTP_PORT),
	_keepAliveTimeout(DEFAULT_KEEP_ALIVE_TIMEOUT, 0),
	_reconnect(false),
	_mustReconnect(false),
	_expectResponseBody(false),
	_pRequestStream(0),
	_pResponseStream(0)
{
}


HTTPClientSession::HTTPClientSession(const SocketAddress& address):
	_host(address.host().toString()),
	_port(address.port()),
	_proxyPort(HTTPSession::HTTP_PORT),
	_keepAliveTimeout(DEFAULT_KEEP_ALIVE_TIMEOUT, 0),
	_reconnect(false),
	_mustReconnect(false),
	_expectResponseBody(false),
	_pRequestStream(0),
	_pResponseStream(0)
{
}


HTTPClientSession::HTTPClientSession(const std::string& host, Poco::UInt16 port):
	_host(host),
	_port(port),
	_proxyPort(HTTPSession::HTTP_PORT),
	_keepAliveTimeout(DEFAULT_KEEP_ALIVE_TIMEOUT, 0),
	_reconnect(false),
	_mustReconnect(false),
	_expectResponseBody(false),
	_pRequestStream(0),
	_pResponseStream(0)
{
}


HTTPClientSession::~HTTPClientSession()
{
	delete _pRequestStream;
	delete _pResponseStream;
}


void HTTPClientSession::setHost(const std::string& host)
{
	if (!connected())
		_host = host;
	else
		throw IllegalStateException("Cannot set the host for an already connected session");
}


void HTTPClientSession::setPort(Poco::UInt16 port)
{
	if (!connected())
		_port = port;
	else
		throw IllegalStateException("Cannot set the port number for an already connected session");
}


void HTTPClientSession::setProxy(const std::string& host, Poco::UInt16 port)
{
	if (!connected())
	{
		_proxyHost = host;
		_proxyPort = port;
	}
	else throw IllegalStateException("Cannot set the proxy host and port for an already connected session");
}


void HTTPClientSession::setProxyHost(const std::string& host)
{
	if (!connected())
		_proxyHost = host;
	else
		throw IllegalStateException("Cannot set the proxy host for an already connected session");
}


void HTTPClientSession::setProxyPort(Poco::UInt16 port)
{
	if (!connected())
		_proxyPort = port;
	else
		throw IllegalStateException("Cannot set the proxy port number for an already connected session");
}


void HTTPClientSession::setKeepAliveTimeout(const Poco::Timespan& timeout)
{
	_keepAliveTimeout = timeout;
}


std::ostream& HTTPClientSession::sendRequest(HTTPRequest& request)
{
	delete _pResponseStream;
	_pResponseStream = 0;

	bool keepAlive = getKeepAlive();
	if ((connected() && !keepAlive) || mustReconnect())
	{
		close();
		_mustReconnect = false;
	}
	if (!connected())
		reconnect();
	if (!keepAlive)
		request.setKeepAlive(false);
	if (!request.has(HTTPRequest::HOST))
		request.setHost(_host, _port);
	if (!_proxyHost.empty())
		request.setURI(proxyRequestPrefix() + request.getURI());
	_reconnect = keepAlive;
	_expectResponseBody = request.getMethod() != HTTPRequest::HTTP_HEAD;
	if (request.getChunkedTransferEncoding())
	{
		HTTPHeaderOutputStream hos(*this);
		request.write(hos);
		_pRequestStream = new HTTPChunkedOutputStream(*this);
	}
	else if (request.getContentLength() != HTTPMessage::UNKNOWN_CONTENT_LENGTH)
	{
		Poco::CountingOutputStream cs;
		request.write(cs);
		_pRequestStream = new HTTPFixedLengthOutputStream(*this, request.getContentLength() + cs.chars());
		request.write(*_pRequestStream);
	}
	else if (request.getMethod() != HTTPRequest::HTTP_PUT && request.getMethod() != HTTPRequest::HTTP_POST)
	{
		Poco::CountingOutputStream cs;
		request.write(cs);
		_pRequestStream = new HTTPFixedLengthOutputStream(*this, cs.chars());
		request.write(*_pRequestStream);
	}
	else
	{
		_pRequestStream = new HTTPOutputStream(*this);
		request.write(*_pRequestStream);
	}	
	_lastRequest.update();
	return *_pRequestStream;
}


std::istream& HTTPClientSession::receiveResponse(HTTPResponse& response)
{
	delete _pRequestStream;
	_pRequestStream = 0;

	do
	{
		response.clear();
		HTTPHeaderInputStream his(*this);
		try
		{
			response.read(his);
		}
		catch (MessageException&)
		{
			if (networkException())
				networkException()->rethrow();
			else
				throw;
		}
	}
	while (response.getStatus() == HTTPResponse::HTTP_CONTINUE);

	_mustReconnect = getKeepAlive() && !response.getKeepAlive();

	if (!_expectResponseBody)
		_pResponseStream = new HTTPFixedLengthInputStream(*this, 0);
	else if (response.getChunkedTransferEncoding())
		_pResponseStream = new HTTPChunkedInputStream(*this);
	else if (response.getContentLength() != HTTPMessage::UNKNOWN_CONTENT_LENGTH)
		_pResponseStream = new HTTPFixedLengthInputStream(*this, response.getContentLength());
	else
		_pResponseStream = new HTTPInputStream(*this);
		
	return *_pResponseStream;
}


int HTTPClientSession::write(const char* buffer, std::streamsize length)
{
	try
	{
		int rc = HTTPSession::write(buffer, length);
		_reconnect = false;
		return rc;
	}
	catch (NetException&)
	{
		if (_reconnect)
		{
			close();
			reconnect();
			int rc = HTTPSession::write(buffer, length);
			_reconnect = false;
			return rc;
		}
		else throw;
	}
}


void HTTPClientSession::reconnect()
{
	if (_proxyHost.empty())
	{
		SocketAddress addr(_host, _port);
		connect(addr);
	}
	else
	{
		SocketAddress addr(_proxyHost, _proxyPort);
		connect(addr);
	}
}


std::string HTTPClientSession::proxyRequestPrefix() const
{
	std::string result("http://");
	result.append(_host);
	result.append(":");
	NumberFormatter::append(result, _port);
	return result;
}


void HTTPClientSession::deleteResponseStream()
{
	delete _pResponseStream;
	_pResponseStream = 0;
}


void HTTPClientSession::deleteRequestStream()
{
	delete _pRequestStream;
	_pRequestStream = 0;
}


void HTTPClientSession::setResponseStream(std::istream* pRespStream)
{
	poco_assert( !_pResponseStream);
	_pResponseStream = pRespStream;
}


void HTTPClientSession::setRequestStream(std::ostream* pRequestStream)
{
	poco_assert (!_pRequestStream);
	_pRequestStream = pRequestStream;
}


bool HTTPClientSession::mustReconnect() const
{
	if (!_mustReconnect)
	{
		Poco::Timestamp now;
		return _keepAliveTimeout <= now - _lastRequest;
	}
	else return true;
}


} } // namespace Poco::Net
