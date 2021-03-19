#include "peerClient.h"
int main() {
    boost::asio::io_service io;
    peerClient(io).start();
    return 0;
}