#include <iostream>
#include <Windows.h>
#include <string>
#include "Parser.h"
using namespace std;

int main() {
    Parser parser;
    parser.LoadFile("test.txt");
    char ip[100];
    int port;

    parser.GetValue("CHAT_SERVER", "IP", ip);
    parser.GetValue("CHAT_SERVER", "PORT", &port);
    printf("ip : %s, port %d\n", ip, port);

    parser.GetValue("CHAT_SERVER", "IP", ip);
    parser.GetValue("CHAT_SERVER", "PORT", &port);
    printf("ip : %s, port %d\n", ip, port);

    parser.GetValue("CHAT_SERVER", "IP", ip);
    parser.GetValue("CHAT_SERVER", "PORT", &port);
    printf("ip : %s, port %d\n", ip, port);
}