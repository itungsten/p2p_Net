#include "peerClient.h"
int main() {
    boost::asio::io_service io;
    peerClient client(io);
    return 0;
}