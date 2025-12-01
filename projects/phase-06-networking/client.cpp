#include <arpa/inet.h>
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <sstream>   

constexpr int PORT = 12345;
constexpr int BUFFER_SIZE = 65536;
constexpr int MODULO = 997;
constexpr int MATRIX_SIZE = 128;
constexpr int MATRIX_ELEMENTS = MATRIX_SIZE * MATRIX_SIZE;

constexpr const char* GROUP_NAME = "radoslav";

int connect_to_server();

bool send_exact(int socket, const char* buffer, size_t length);

bool send_group_name(int socket);

bool read_matricies_from_server(int socket, int* a, int* b);

int trace_ab(const int* a, const int* b);

bool send_result_to_server(int socket, int trace);

int main() {
    while (true) {
        int status = connect_to_server();

        std::cerr << "Connection ended (status " << status
                  << "). Reconnecting in 1 second...\n";

        sleep(1);
    }

    return 0;
}


int connect_to_server() {
    int sock = 0;
    sockaddr_in serv_addr{};

    int a[MATRIX_ELEMENTS] = {0};
    int b[MATRIX_ELEMENTS] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation error" << std::endl;
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/Address not supported" << std::endl;
        close(sock);
        return -1;
    }

    if (connect(sock, (sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection Failed" << std::endl;
        close(sock);
        return -1;
    }

    // std::cout << "Connected to server.\n";

    if (!send_group_name(sock)) {
        std::cerr << "Failed to send group name to server" << std::endl;
        close(sock);
        return -1;
    }

    // std::cout << "Sent group name.\n";


    while (true) {
        if (!read_matricies_from_server(sock, a, b)) {
            std::cerr << "Failed to read matrices (server closed or error).\n";
            break;
        }

        int trace = trace_ab(a, b);

        if (!send_result_to_server(sock, trace)) {
            std::cerr << "Failed to send result to server.\n";
            break;
        }

        
    }

    close(sock);
    // std::cout << "Disconnected from server.\n";
    return -1;
}


bool send_exact(int socket, const char* buffer, size_t length) {
    size_t totalSent = 0;
    while (totalSent < length) {
        ssize_t bytesSent = send(socket, buffer + totalSent,
                                 length - totalSent, 0);
        if (bytesSent <= 0) {
            return false;
        }
        totalSent += static_cast<size_t>(bytesSent);
    }
    return true;
}

bool send_group_name(int socket) {
    std::string name = std::string(GROUP_NAME) + "\n";
    return send_exact(socket, name.c_str(), name.size());
}

bool read_matricies_from_server(int socket, int* a, int* b) {
    std::string data;
    data.reserve(200000); 

    char buf[BUFFER_SIZE];
    int newline_count = 0;

    // The server sends exactly 4 newline-terminated lines per challenge:
    // cid\n
    // N\n
    // A values ...\n
    // B values ...\n
    while (newline_count < 4) {
        ssize_t bytes = recv(socket, buf, sizeof(buf), 0);
        if (bytes <= 0) {
            return false;
        }

        data.append(buf, bytes);

        for (ssize_t i = 0; i < bytes; ++i) {
            if (buf[i] == '\n') {
                ++newline_count;
            }
        }
    }

    std::istringstream iss(data);

    int cid = 0;
    int N = 0;

    if (!(iss >> cid)) {
        std::cerr << "Failed to parse challenge id" << std::endl;
        return false;
    }

    if (!(iss >> N)) {
        std::cerr << "Failed to parse matrix size" << std::endl;
        return false;
    }

    if (N != MATRIX_SIZE) {
        std::cerr << "Warning: server N = " << N
                  << " but client MATRIX_SIZE = " << MATRIX_SIZE << std::endl;
    }

    for (int i = 0; i < MATRIX_ELEMENTS; ++i) {
        if (!(iss >> a[i])) {
            std::cerr << "Failed to parse A[" << i << "]" << std::endl;
            return false;
        }
    }

    for (int i = 0; i < MATRIX_ELEMENTS; ++i) {
        if (!(iss >> b[i])) {
            std::cerr << "Failed to parse B[" << i << "]" << std::endl;
            return false;
        }
    }

    // std::cout << "Got challenge " << cid << " with N=" << N << std::endl;
    // std::cout << "A[0] = " << a[0] << ", B[0] = " << b[0] << std::endl;

    return true;
}

int trace_ab(const int* a, const int* b) {
    long long acc = 0;  

    for (int i = 0; i < MATRIX_SIZE; ++i) {
        int rowA = i * MATRIX_SIZE;
        for (int k = 0; k < MATRIX_SIZE; ++k) {
            int a_ik = a[rowA + k];                
            int b_ki = b[k * MATRIX_SIZE + i];    
            acc += static_cast<long long>(a_ik) * b_ki;
        }
    }

    int result = static_cast<int>(acc % MODULO);
    if (result < 0) result += MODULO;
    return result;
}

bool send_result_to_server(int socket, int trace) {
    int answer = trace % MODULO;
    std::string trace_str = std::to_string(answer) + "\n";
    return send_exact(socket, trace_str.c_str(), trace_str.size());
}
