#include "HttpClient.h"
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <iostream>

void WriteHandler(const boost::system::error_code& ec, std::size_t bytes_transferred)
{
	BOOST_LOG_TRIVIAL(error) << ec.message();
}





int main()
{

	boost::asio::io_service service;
	boost::asio::io_service::work work(service);

	HTTPClient* hc = new HTTPClient(service, "ropx01", 8080);

	hc->sendRequest("GET http://felixastner.de/ HTTP/1.0\r\nHost:felixastner.de\r\n\r\n");


	//service.run();

	std::cout << &(hc->getBuffer());

	return 0;
}
