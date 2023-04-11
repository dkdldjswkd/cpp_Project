#include <iostream>
#include <Windows.h>
#include <string>
#include "Parser.h"
using namespace std;
#include <cstdio>
#include <cstdarg>

void A(const char* queryFormat, va_list args) {
    vprintf(queryFormat, args);
}

void B(const char* queryFormat, ...) {
    va_list args;
    va_start(args, queryFormat);
    A(queryFormat, args);
    va_end(args);
}

int main() {
    B("data : %d\n", 3);
    return 0;
}

//Parser parser;
//parser.LoadFile("test.txt");
//char ip[100];
//int port;

//parser.GetValue("CHAT_SERVER", "IP", ip);
//parser.GetValue("CHAT_SERVER", "PORT", &port);
//printf("ip : %s, port %d\n", ip, port);

//parser.GetValue("CHAT_SERVER", "IP", ip);
//parser.GetValue("CHAT_SERVER", "PORT", &port);
//printf("ip : %s, port %d\n", ip, port);

//parser.GetValue("CHAT_SERVER", "IP", ip);
//parser.GetValue("CHAT_SERVER", "PORT", &port);
//printf("ip : %s, port %d\n", ip, port);
