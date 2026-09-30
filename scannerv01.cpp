// Create a socketaddr with local ip and AF_INET
#include <iostream>
#include <string>
#include <cerrno>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char *argv[])
{

    if (argc != 4)
    {
        std::cerr << "Error. Usage: scanner <target> <start_port> <end_port>\n";
        return 1;
    }
    int start = std::stoi(argv[2]);
    int end  = std::stoi(argv[3]);

    sockaddr_in target{};
    target.sin_family = AF_INET;
    if(inet_pton(AF_INET, argv[1], &target.sin_addr) != 1)
    {
        std::cerr << "Invalid IPv4 Adress: " << argv[1] << '\n';
        return 1;
    }

    for (int port {start}; port <= end; port ++)
    {
       
        target.sin_port = htons(port);

        int sd = socket(target.sin_family, SOCK_STREAM, 0);

        if(sd == -1)
        {
            std::cerr << "socket "<< std::strerror(errno) << '\n';
            return 1;
        }
        timeval tv{};
        tv.tv_sec = 2;                                       // wait at most 2 seconds
        setsockopt(sd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof tv);

        int cd = connect(sd, reinterpret_cast<sockaddr*>(&target), sizeof target);
        if (cd == 0)
        {
            std::cout << "port: " << port << " open\n";
        }
        else if(errno == ECONNREFUSED){//closed
            }
        else{// filtered
             }
        close(sd);
        
    }
    return 0;
}
