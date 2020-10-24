#include <iostream>
#include <iomanip>
#include <algorithm>
#include <string>
#include <cerrno>
#include <map>

#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_EVENTS  10
#define MAX_BUFFER  1024

#define INVALID_FD  -1
#define ENOERR      0

static int listenSocket = INVALID_FD;
static std::map<int, std::string> connSockets;

static bool setNonBlocking(int fd, bool blocking) {

   if (fd < 0) return false;
#ifdef _WIN32
   unsigned long mode = blocking ? 0 : 1;
   return (ioctlsocket(fd, FIONBIO, &mode) == 0) ? true : false;
#else
   int flags = fcntl(fd, F_GETFL, 0);
   if (flags == -1) return false;
   flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
   return (fcntl(fd, F_SETFL, flags) == 0) ? true : false;
#endif
}

static void sigint_handler(int signum) {

    std::cout << std::endl;

    for (auto it = connSockets.begin(); it != connSockets.end(); ++it) {
        shutdown(it->first, SHUT_RDWR);                                            
        close(it->first);
        std::cout << "terminated: " << it->first << std::endl;
    }

    std::cout << "terminated: " << listenSocket << std::endl;
    close(listenSocket);

    exit(EXIT_SUCCESS);
}


int main(int argc, const char** argv) {

    int rc = ENOERR;
    struct sockaddr_in listenSocketAddr = {0};

    struct sigaction sa = {sigint_handler};
    sigaction(SIGINT, &sa, NULL);

    while(true) {

        const char *delim = 0;

        if ((argc != 2) || (NULL == (delim = strchr(argv[1], ':')))) {

            std::string arg = argv[0];
            std::cout << "Usage: "
                    << arg.substr(arg.find_last_of("/\\") + 1)
                    << " [<address>]:<port>"
                    << std::endl;
            rc = EINVAL;
            break;
        }
        else {

            std::string argAddr = std::string(argv[1], delim - &argv[1][0]);
            std::string argPort = &delim[1];

            if (argAddr.empty()) {
                listenSocketAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);            
            }
            else {

                if(inet_aton(argAddr.c_str(), &listenSocketAddr.sin_addr) == 0) {
                    std::cout << "Invalid ip address: "
                            << argAddr << std::endl;
                    rc = EINVAL;
                    break;
                }
            }

            if ((argPort.empty()) || (argPort.length() > 5) ||
                (!std::all_of(argPort.begin(), argPort.end(), ::isdigit))) {

                std::cout << "Invalid port number: " << argPort << std::endl;
                rc = EINVAL;
                break;
            }
            else {

                int port = atoi(argPort.c_str());
                if (port <= 0x400 || port > 0xFFFF) {
                    std::cout << "Invalid port value, allowed range is "
                            << "[" << 0x400 << "-" << 0xFFFF << "]: "
                            << argPort << std::endl;
                    rc = EINVAL;
                    break;
                }
                
                listenSocketAddr.sin_port = htons((uint16_t)port);
            }
           
            listenSocketAddr.sin_family = AF_INET;
        }

        listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
        if (listenSocket == INVALID_FD) {
            std::cout << "Failed to create socket: "
                    << strerror(errno) << " (rc=" << errno << ")"
                    << std::endl;
            rc = errno;
            break;
        }

        const int optval = 1;
        if (ENOERR != setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int))) {
            std::cout << "Failed to set socket options: "
                    << strerror(errno) << " (rc=" << errno << ")"
                    << std::endl;
            rc = errno;
            break;
        }

        if (ENOERR != bind(listenSocket, 
                           (const sockaddr*) &listenSocketAddr,
                           sizeof(listenSocketAddr)) ) {
            std::cout << "Failed to assign network address: "
                    << strerror(errno) << " (rc=" << errno << ")"
                    << std::endl;
            rc = errno;
            break;
        }

        if (ENOERR != listen(listenSocket, SOMAXCONN)) {
            std::cout << "Failed to mark socket as passive: "
                    << strerror(errno) << " (rc=" << errno << ")"
                    << std::endl;
            rc = errno;
            break;
        }

        break;
    }

    if (rc == ENOERR) {

        int epfd = INVALID_FD;
            
        epfd = epoll_create1(0);
        if (epfd == INVALID_FD) {
            std::cout << "Failed to create epoll instance: "
                << strerror(errno) << " (rc=" << errno << ")"
                << std::endl;
            rc = errno;
        }
        else {

            char buffer[MAX_BUFFER] = {0};
            struct epoll_event ev, events[MAX_EVENTS];

            ev.events = EPOLLIN;
            ev.data.fd = listenSocket;
            
            if (ENOERR != epoll_ctl(epfd, EPOLL_CTL_ADD, listenSocket, &ev)) {
                std::cout << "Failed to register local epoll descriptor: "
                    << strerror(errno) << " (rc=" << errno << ")"
                    << std::endl;
                rc = errno;
            }

            while(rc == ENOERR) {

                int noOfEvents = epoll_wait(epfd, events, MAX_EVENTS, -1);
                if (noOfEvents == -1) {
                    std::cout << "Failed to wait for epoll events: "
                        << strerror(errno) << " (rc=" << errno << ")"
                        << std::endl;
                    rc = errno;
                    break;
                }

                for(int eventIx = 0; eventIx < noOfEvents; ++eventIx) {

                    if (events[eventIx].data.fd == listenSocket) {

                        struct sockaddr_in connSocketAddr = {0};
                        socklen_t socklen = sizeof(connSocketAddr);

                        int connSocket = accept(listenSocket, (struct sockaddr*)&connSocketAddr, &socklen);
                        
                        if (connSocket == INVALID_FD) {
                            std::cout << "Failed to accept connetion: "
                                      << strerror(errno) << " (rc=" << errno << ")"
                                      << std::endl;
                            rc = errno;
                            break;
                        }
                        
                        setNonBlocking(connSocket, true);

                        ev.events = EPOLLIN | EPOLLET;
                        ev.data.fd = connSocket;

                        if (ENOERR != epoll_ctl(epfd, EPOLL_CTL_ADD, connSocket, &ev)) {
                            std::cout << "Failed to register incoming connection: "
                                     << strerror(errno) << " (rc=" << errno << ")"
                                     << std::endl;
                            rc = errno;
                            break;
                        }

                        inet_ntop(connSocketAddr.sin_family, &connSocketAddr.sin_addr, buffer, socklen);
                        connSockets[connSocket] = std::string(buffer) + std::string(":") 
                                                + std::to_string(connSocketAddr.sin_port);

                        std::string output = connSockets[connSocket] + " ++ connected\n\r";
                        for (auto it = connSockets.begin(); it != connSockets.end(); it++) {
                           if ((it->first != listenSocket) &&
                               (it->first != connSocket)) {

                                send(it->first, output.c_str(), strlen(output.c_str()), MSG_DONTWAIT);
                            }
                        }
                        std::cout << output;
                    }
                    else {

                        int nres = ENOERR;
                        while((nres = recv(events[eventIx].data.fd, buffer, MAX_BUFFER - 1, MSG_NOSIGNAL | MSG_DONTWAIT)) > 0) {

                            buffer[nres] = '\0';

                            std::string output = connSockets[events[eventIx].data.fd] + " >> " + buffer;
                            for (auto it = connSockets.begin(); it != connSockets.end(); it++) {
                                if ((it->first != listenSocket) &&
                                    (it->first != events[eventIx].data.fd)) {

                                    send(it->first, output.c_str(), strlen(output.c_str()), MSG_DONTWAIT);
                                }
                            }
                            std::cout << output;
                        }
                    
                       if ((nres == -1) && (errno != EAGAIN)) {

                            std::string output = connSockets[events[eventIx].data.fd] + " -- disconnected\n\r";
                            for (auto it = connSockets.begin(); it != connSockets.end(); it++) {
                                if ((it->first != listenSocket) &&
                                    (it->first != events[eventIx].data.fd)) {

                                    send(it->first, output.c_str(), strlen(output.c_str()), MSG_DONTWAIT);
                                }
                            }
                            std::cout << output;

                            connSockets.erase(events[eventIx].data.fd);

                            shutdown(events[eventIx].data.fd, SHUT_RDWR);
                            close(events[eventIx].data.fd);
                        }
                    }
                }
            }
        }
    }

    if (listenSocket != INVALID_FD) {
        close(listenSocket);
    }

    return rc;
}