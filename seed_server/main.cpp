#include "Server.h"

int main() {
    boost::asio::io_service io;
    Server server(io);
    io.run();
    return 0;
}