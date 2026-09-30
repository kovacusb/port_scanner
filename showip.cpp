#include <iostream>
#include <memory>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/time.h>

int main(int argc, char *argv[])
{
    if (argc != 2) {
        std::cerr << "usage: showip hostname\n";
        return 1;
    }

    addrinfo hints{};                  // {} zero-initializes, replaces memset
    hints.ai_family = AF_UNSPEC;       // Either IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;

    addrinfo *raw = nullptr;
    int status = getaddrinfo(argv[1], nullptr, &hints, &raw);
    if (status != 0) {
        std::cerr << "getaddrinfo: " << gai_strerror(status) << '\n';
        return 2;
    }

    // RAII: freeaddrinfo() runs automatically when 'res' goes out of scope
    std::unique_ptr<addrinfo, decltype(&freeaddrinfo)> res(raw, freeaddrinfo);

    std::cout << "IP addresses for " << argv[1] << ":\n\n";

    for (addrinfo *p = res.get(); p != nullptr; p = p->ai_next) {
        const void *addr;
        const char *ipver;

        // get the pointer to the address itself,
        // different fields in IPv4 and IPv6:
        if (p->ai_family == AF_INET) {  // IPv4
            auto *ipv4 = reinterpret_cast<sockaddr_in *>(p->ai_addr);
            addr = &ipv4->sin_addr;
            ipver = "IPv4";
        } else {                        // IPv6
            auto *ipv6 = reinterpret_cast<sockaddr_in6 *>(p->ai_addr);
            addr = &ipv6->sin6_addr;
            ipver = "IPv6";
        }

        // convert the IP to a string and print it:

        char ipstr[INET6_ADDRSTRLEN];
        inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
        std::cout << "  " << ipver << ": " << ipstr << '\n';
    }

    return 0;   // no manual freeaddrinfo() needed
}