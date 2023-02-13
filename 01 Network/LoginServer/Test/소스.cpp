#include <iostream>
#include <cstdarg>

int find_sum(int sentinel, ...) {
    int sum = 0;
    va_list args;
    va_start(args, sentinel);
    int arg = va_arg(args, int);
    while (arg != sentinel) {
        sum += arg;
        arg = va_arg(args, int);
    }
    va_end(args);
    return sum;
}

int main() {
    std::cout << find_sum(1) << std::endl;
    return 0;
}