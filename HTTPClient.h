#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <boost/lexical_cast.hpp>
#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>
#include <boost/bind.hpp>

namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace attrs = boost::log::attributes;
namespace keywords = boost::log::keywords;

class HTTPClient
{
	public:
		HTTPClient(boost::asio::io_service& service,
					const std::string& remoteHost, unsigned int port);

		virtual ~HTTPClient();

		bool isOpen();
		void closeConnection();


        void sendRequest(const std::string& data);

		boost::asio::streambuf& getBuffer();

	protected:
	private:
		std::string remote_host_;

		boost::asio::io_service io_service_;
		boost::asio::ip::tcp::socket socket_;

		boost::asio::streambuf response_;
		boost::asio::streambuf content_response_;

		void writeHandler(const boost::system::error_code& ec);
        void readUntilHandler(const boost::system::error_code& ec);
        void readStatusLineHandler(const boost::system::error_code& ec);
        void readHeadersHandler(const boost::system::error_code& ec);
        void readContentHandler(const boost::system::error_code& ec);
};

#endif // HTTPCLIENT_H
