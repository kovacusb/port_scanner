// Create a socketaddr with local ip and AF_INET
#include <iostream>
#include <string>
#include <cerrno>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include <atomic>
#include <thread>
#include <vector>

// Socket Class for RAAI
class Socket{
    public:
        Socket(int domain, int type, int protocol): m_sd(socket(domain, type, protocol)){}
        ~Socket(){if(m_sd != -1) close(m_sd);}

        // Deleting copy assignments
        Socket(const Socket&) = delete;
        Socket &operator = (const Socket&) = delete;
        
        int get_sd()const{return m_sd;}
        bool valid()const{return m_sd != -1;}

    private:
        int m_sd{};
};

// Define struct for possible states of a port
enum class PortState{Open, Closed, Filtered, Error};
// Define function probe:PortState
PortState probe(sockaddr_in portaddr, int port)
{

        Socket sock{AF_INET, SOCK_STREAM, 0};
        if (!sock.valid())
        {
            return PortState::Error;
        }
      
        timeval tv{};
        tv.tv_sec = 2;                                       // wait at most 2 seconds
        setsockopt(sock.get_sd(), SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof tv);

        portaddr.sin_port = htons(port);

        int statusConnect = connect(sock.get_sd(), reinterpret_cast<sockaddr*>(&portaddr), sizeof portaddr);

        if (statusConnect == 0)
        {
            return PortState::Open;
        }
        else if(errno == ECONNREFUSED){//closed

            return PortState::Closed;
        }
        else{// filtered
            return PortState::Filtered;
        }       
}
//Define tostring:char*

int main(int argc, char *argv[])
{

    if (argc != 4)
    {
        std::cerr << "Error. Usage: scanner <target> <start_port> <end_port>\n";
        return 1;
    }
    int start, end;
    try{
        start = std::stoi(argv[2]);
        end = std::stoi(argv[3]);
    }
    catch(const std::exception &e){
        std::cerr << "Invalid Port Number: " << e.what() << '\n';
        return 1;
    }
    
    if ((start < 1) || (end > 65535) || end < start)
    {
        std::cerr << "Invalid port number\n";
        return 1;
    }

    sockaddr_in target{};
    target.sin_family = AF_INET;

    if(inet_pton(AF_INET, argv[1], &target.sin_addr) != 1)
    {
        std::cerr << "Invalid IPv4 Adress: " << argv[1] << '\n';
        return 1;
    }

    // Atomic integer, mutex to write in a vector and buffer vectors
    std::atomic<int>nextPort{start};
    std::vector<PortState> results(end - start + 1);

    // Lambda function worker
    auto worker = [&](){
    while (true){
        
        int port = nextPort.fetch_add(1);
        if (port > end) break;

        results[port - start] = probe(target, port);
        }
       
    };

    std::vector<std::thread> threads;
    for (int t = 0; t < 50; t++)
    {
        threads.emplace_back(worker);
    }

    for(auto &t: threads)
    {
        t.join();
    }

    
    int filteredPorts{};
    int closedPorts{};
    for (int i = 0; i < static_cast<int>(results.size()); i++)
    {
        if(results[i] == PortState::Open)
        {
            std::cout << "Port " << i + 1 << " Open\n";
        }

        else if (results[i] == PortState::Filtered)
        {
            std::cout << "Port " << i + 1 << " Filtered\n";
        }
        else if(results[i] == PortState::Filtered) filteredPorts++;
        else if(results[i] == PortState::Closed) closedPorts++;
    }

    std::cout << "Closed ports: " << closedPorts << '\n';
    std::cout << "Filtered ports: " << filteredPorts << '\n';
}
