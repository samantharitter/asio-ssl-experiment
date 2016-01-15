// an experiment to see if we can use ASIO with LibreSSL.

#include <asio.hpp>
#include <asio/ssl.hpp>

#include <stdio.h>
#include <string>
#include <vector>

using asio::ip::tcp;
using asio::ssl::context;

//--------------------------------------------------------------

class server_session {
public:
  server_session(asio::io_context *context, asio::ssl::context *ssl_context)
      : _ssl_stream(*context, *ssl_context) {}

  void do_read() {
    try {
      std::cout << "got a new connection" << std::endl;

      // read message
      char buf[5]; // message will be "hello\n"

      asio::error_code ec;
      std::size_t n = _ssl_stream.read_some(asio::buffer(buf, 5));

      std::cout << "read " << n << " bytes: " << buf << std::endl;

      // respond?  do something useful, close connection

    } catch (const std::exception &e) {
      std::cout << "Exception: " << e.what() << std::endl;
    }
  }

  void do_handshake() {
    _ssl_stream.async_handshake(
        asio::ssl::stream_base::server, [this](std::error_code ec) {
          if (ec) {
            std::cout << "handshake error: " << ec.message() << std::endl;
            return;
          }

          do_read();
        });
  }

  asio::ssl::stream<tcp::socket> *stream() { return &_ssl_stream; }

private:
  asio::ssl::stream<asio::ip::tcp::socket> _ssl_stream;
};

//--------------------------------------------------------------

class server {
public:
  server(char *port)
      : _context(), _ssl_context(context::sslv23),
        _endpoint(tcp::v4(), std::atoi(port)), _acceptor(_context, _endpoint) {
    _ssl_context.use_tmp_dh_file("dh512.pem");
    _ssl_context.set_options(context::default_workarounds | context::no_sslv2 |
                             context::single_dh_use);
    _ssl_context.use_certificate_file("newcert.pem", context::pem);
    _ssl_context.use_private_key_file("privkey.pem", context::pem);
  }

  void run() {
    listen();
    _context.run();
  }

  void listen() {

    std::unique_ptr<server_session> new_session(
        new server_session(&_context, &_ssl_context));

    _sessions.push_back(std::move(new_session));

    _acceptor.async_accept(_sessions.back()->stream()->lowest_layer(),
                           [this](std::error_code ec) {
                             if (!ec) {
                               _sessions.back()->do_handshake();
                             }

                             listen();

                             // todo clean up old sessions
                           });
  }

private:
  asio::io_context _context;
  asio::ssl::context _ssl_context;

  tcp::endpoint _endpoint;
  tcp::acceptor _acceptor;

  std::vector<std::unique_ptr<server_session>> _sessions;
};

//--------------------------------------------------------------

int main(int argc, char *argv[]) {
  try {
    // take port from cmd line
    if (argc != 2) {
      std::cerr << "Usage: ssl_server <port>\n";
      return 1;
    }

    server server{argv[1]};
    server.run();

    return 0;

  } catch (const std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
    return 1;
  }
}
