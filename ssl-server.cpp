// an experiment to see if we can use ASIO with LibreSSL.

#include <asio.hpp>
#include <asio/ssl.hpp>

#include <stdio.h>
#include <string>

using asio::ip::tcp;

void handle_connection(tcp::socket socket) {
  try {
    std::cout << "got a new connection" << std::endl;

    // read message
    char buf[128];
    auto res = asio::read(socket, asio::buffer(buf, 128));

    std::cout << "got message: " << buf << std::endl;

    // respond

    // close connection
    // do something useful
  } catch (const std::exception &e) {
    std::cout << "Exception: " << e.what() << std::endl;
  }
}

void listen(tcp::acceptor &acceptor) {
  acceptor.async_accept([&acceptor](std::error_code ec, tcp::socket socket) {
    if (!ec) {
      handle_connection(std::move(socket));
    }

    listen(acceptor);
  });
}

int main(int argc, char *argv[]) {
  try {
    // take host and port from cmd line
    if (argc != 2) {
      std::cerr << "Usage: ssl_server <port>\n";
      return 1;
    }

    auto port = argv[1];

    asio::io_context context{};

    // establish endpoint
    tcp::endpoint endpoint(tcp::v4(), std::atoi(port));
    tcp::acceptor acceptor(context, endpoint);

    // start listening
    listen(acceptor);

    context.run();

    return 0;

  } catch (const std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
    return 1;
  }
}
