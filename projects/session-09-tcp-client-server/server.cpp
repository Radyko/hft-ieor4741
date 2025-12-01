#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sstream>
#include <vector>
#include <string>
#include <cstring>

struct Order {
    std::string type;
    std::string symbol;
    int quantity;
    double price;
};

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in address = {};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8888);

    bind(server_fd, (sockaddr*)&address, sizeof(address));
    listen(server_fd, 3);
    std::cout << "Server listening on port 8888\n";

    socklen_t addrlen = sizeof(address);
    int client_fd = accept(server_fd, (sockaddr*)&address, &addrlen);

    char buffer[1024] = {0};
    read(client_fd, buffer, sizeof(buffer) - 1);
    std::cout << "Received: " << buffer << std::endl;

    std::istringstream ss(buffer);
    std::string type, symbol;
    int quantity;
    double price;

    std::vector<Order> orders;
    std::string reply;

    if (ss >> type >> symbol >> quantity >> price) {
        Order newOrder{type, symbol, quantity, price};
        orders.push_back(newOrder);
        reply = "Order received and stored.\n";
    } else {
        reply = "ERROR: Invalid format.\n";
    }

    send(client_fd, reply.c_str(), reply.size(), 0);

    std::cout << "\nStored orders:\n";
    for (const auto& o : orders) {
        std::cout << o.type << " " << o.symbol << " "
                  << o.quantity << " " << o.price << std::endl;
    }

    close(client_fd);
    close(server_fd);
    return 0;
}
