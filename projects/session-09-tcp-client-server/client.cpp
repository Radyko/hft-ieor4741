#include <cstring>
#include <iostream>
#include <ostream>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>


int main()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serv_addr = {};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8888);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr.s_addr);
    int connect_return = connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (connect_return < 0)
    {
        perror("connect");
    }
    std::string msg;
    std::cout << "Enter your order in the following format:" << std::endl;
    std::cout << "ORDER_TYPE SYMBOL QTY PRICE" << std::endl;
    std::getline(std::cin, msg);
    const char * c_msg = msg.c_str();
    send(sock, c_msg, strlen(c_msg), 0);
    char buffer[1024] = {};
    read(sock, buffer, 1024);
    std::cout << "The client sent " << buffer << std::endl;
    close(sock);
    return 0;
}
