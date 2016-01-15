// an experiment to see if we can use ASIO with LibreSSL.

#include <asio.hpp>
#include <asio/ssl.hpp>

#include <stdio.h>
#include <string>

using asio::ip::tcp;

int main(int argc, char *argv[]) {
  try {
    // take host and port from cmd line
    if (argc != 3) {
      std::cerr << "Usage: ssl_client <host> <port>\n";
      return 1;
    }

    auto host = argv[1];
    auto port = argv[2];

    asio::io_context context{};

    // set up an ssl_stream
    asio::ssl::context ssl_context{asio::ssl::context::sslv23};
    asio::ssl::stream<asio::ip::tcp::socket> ssl_stream{context, ssl_context};

    // resolve endpoints for our server
    tcp::resolver resolver{context};
    auto endpoints = resolver.resolve(host, port);

    // synchronously connect
    auto res2 = asio::connect(ssl_stream.lowest_layer(), endpoints);

    // handshake
    ssl_stream.handshake(asio::ssl::stream_base::handshake_type::client);

    // validate certificate
    // gonna skip this for now.

    // send a message
    std::string msg{"hello\n"};
    auto res = asio::write(ssl_stream, asio::buffer(msg.data(), msg.length()));

    return 0;

  } catch (const std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
    return 1;
  }
}
