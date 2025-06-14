#include <server.h>

int main() {
    Server server("config.txt");
    server.run();
    return 0;
}