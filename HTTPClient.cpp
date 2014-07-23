#include "HTTPClient.h"

HTTPClient::HTTPClient(boost::asio::io_service& service,
					const std::string& remoteHost, unsigned int port)
: io_service_()
, socket_(io_service_)
{
/*
	boost::asio::socket_base::debug option(true);
	socket_.set_option(option);
*/

	boost::asio::ip::tcp::resolver r(io_service_);
	boost::asio::ip::tcp::resolver::query q(remoteHost,
					boost::lexical_cast<std::string>(port));
	boost::system::error_code ec;

	boost::asio::connect(socket_, r.resolve(q), ec);

	BOOST_LOG_TRIVIAL(debug) << "open connection";

	if(ec)
	{
		//an error occured
		BOOST_LOG_TRIVIAL(error) << ec.message();
	}
	else
	{
		BOOST_LOG_TRIVIAL(info) << ec.message();
	}
}

HTTPClient::~HTTPClient()
{
	this->closeConnection();
}

bool HTTPClient::isOpen()
{
	return socket_.is_open();
}

void HTTPClient::closeConnection()
{
	if(socket_.is_open())
	{
		socket_.close();
		boost::system::error_code ec;

		BOOST_LOG_TRIVIAL(debug) << "closing connection";

		if(ec)
			BOOST_LOG_TRIVIAL(error) << ec.message();
		else
			BOOST_LOG_TRIVIAL(info) << ec.message();
	}
}

#include <iostream>
void HTTPClient::sendRequest(const std::string& data)
{
	if(socket_.is_open())
	{
		BOOST_LOG_TRIVIAL(debug) << "send request";

		boost::asio::async_write(socket_,
				boost::asio::buffer(data, data.size()),
				boost::bind(
					&HTTPClient::writeHandler,
					this,
					boost::asio::placeholders::error));
		socket_.get_io_service().poll_one();
	}
}

void HTTPClient::writeHandler(const boost::system::error_code& ec)
{
	BOOST_LOG_TRIVIAL(debug) << "handling write request";

	if(!ec)
	{
		BOOST_LOG_TRIVIAL(info) << ec.message();

		boost::asio::async_read_until(socket_, response_, "\r\n",
				boost::bind(
					&HTTPClient::readStatusLineHandler,
					this,
					boost::asio::placeholders::error));
		socket_.get_io_service().run_one();

	}
	else
	{
		BOOST_LOG_TRIVIAL(error) << ec.message();
	}
}




void HTTPClient::readStatusLineHandler(const boost::system::error_code& ec)
{
	BOOST_LOG_TRIVIAL(debug) << "handling read status line";

	if(!ec)
	{
		BOOST_LOG_TRIVIAL(info) << ec.message();

		std::istream response_stream(&response_);
		std::string http_version;

		response_stream >> http_version;

		unsigned int status_code;
		response_stream >> status_code;

		std::string status_message;
		std::getline(response_stream, status_message);
		if(!response_stream || http_version.substr(0, 5) != "HTTP/")
		{
			BOOST_LOG_TRIVIAL(error) << "Invalid http response";
			return;
		}
		if(status_code != 200)
		{
			BOOST_LOG_TRIVIAL(error) << "Response status code is " << status_code;
			return;
		}

		boost::asio::async_read_until(socket_, response_, "\r\n\r\n",
				boost::bind(&HTTPClient::readHeadersHandler,
								this,
								boost::asio::placeholders::error));
		socket_.get_io_service().run_one();

	}
	else
	{
		BOOST_LOG_TRIVIAL(error) << ec.message();
	}
}



void HTTPClient::readHeadersHandler(const boost::system::error_code& ec)
{
	BOOST_LOG_TRIVIAL(debug) << "handling read status line";

	if(!ec)
	{
		BOOST_LOG_TRIVIAL(info) << ec.message();


		std::istream response_stream(&response_);
		std::string header;

		while (std::getline(response_stream, header) && header != "\r")
			//std::cout << header << "\n";
			response_.consume(header.length());
		//std::cout << "\n";
/*
		// Write whatever content we already have to output.
		if (response_.size() > 0)
	    	std::cout << &response_;
*/
		boost::asio::async_read(socket_, response_,
			boost::asio::transfer_at_least(1),
			boost::bind(&HTTPClient::readContentHandler,
						this,
						boost::asio::placeholders::error));
		socket_.get_io_service().run_one();

	}
	else
	{
		BOOST_LOG_TRIVIAL(error) << ec.message();
	}
}


void HTTPClient::readContentHandler(const boost::system::error_code& ec)
{
	BOOST_LOG_TRIVIAL(debug) << "handling read content";

	if(!ec)
	{
		BOOST_LOG_TRIVIAL(info) << ec.message();

		boost::asio::async_read(socket_, response_,
			boost::asio::transfer_at_least(1),
			boost::bind(&HTTPClient::readContentHandler,
						this,
						boost::asio::placeholders::error));
		socket_.get_io_service().run_one();

	}
	else if(ec != boost::asio::error::eof)
	{
		BOOST_LOG_TRIVIAL(debug) << ec.message();
	}
	else
	{
		BOOST_LOG_TRIVIAL(error) << ec.message();
	}
}


boost::asio::streambuf& HTTPClient::getBuffer()
{
	BOOST_LOG_TRIVIAL(trace) << "returned buffer";
	return response_;
}
