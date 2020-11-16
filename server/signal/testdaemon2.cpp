#include <stdlib.h>
#include <unistd.h>

#include <iostream>

int main(int argc, char* argv[])
{
    if (daemon(0, 0) == -1) {
        std::cout << "error" << std::endl;
        exit(-1);
    }
    while (1) {
        sleep(1);
    }
}