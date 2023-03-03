#include <WS2tcpip.h>
#include <WinSock2.h>
#include <memory.h>
#include <timeapi.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Winmm.lib")

#define IP "12"

void main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    //inet_pton(AF_INET, INADDR_ANY, &addr.sin_addr);
    //addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    //addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    }
}