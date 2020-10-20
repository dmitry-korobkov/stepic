#include <iostream>
#include <iomanip>
#include <algorithm>
#include <string>
#include <cerrno>
#include <set>

#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_EVENTS  10
#define MAX_BUFFER  5

#define INVALID_FD  -1
#define ENOERR      0

static int listenSocket = INVALID_FD;
static std::set<int> connSockets;

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
        shutdown(*it, SHUT_RDWR);                                            
        close(*it);
        std::cout << "terminated: " << *it << std::endl;
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

        if (argc != 2) {

            std::string arg = argv[0];
            std::cout << "Usage: "
                    << arg.substr(arg.find_last_of("/\\") + 1)
                    << " <port>"
                    << std::endl;
            rc = EINVAL;
            break;
        }
        else {

            int port;
            std::string arg = argv[1];

            if ((arg.empty()) || (arg.length() > 5) ||
                (!std::all_of(arg.begin(), arg.end(), ::isdigit))) {

                std::cout << "Invalid port number: " << arg << std::endl;
                rc = EINVAL;
                break;
            }

            port = atoi(arg.c_str());
            if (port <= 0x400 || port > 0xFFFF) {
                std::cout << "Invalid port value, allowed range is "
                        << "[" << 0x400 << "-" << 0xFFFF << "]: "
                        << arg << std::endl;
                rc = EINVAL;
                break;
            }

            listenSocketAddr.sin_family = AF_INET;
            listenSocketAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
            listenSocketAddr.sin_port = htons((uint16_t)port);
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

            struct epoll_event ev, events[MAX_EVENTS];
            std::string buffer(MAX_BUFFER, '\0');

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
                    
                        int connSocket = accept(listenSocket, NULL, NULL);
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

                       connSockets.insert(connSocket);
                       std::cout << std::setw(2) << connSocket
                                 << " == connected"
                                 << std::endl;
                    }
                    else {

                        int nres = ENOERR;
                        while((nres = recv(events[eventIx].data.fd, &buffer[0], buffer.capacity(), MSG_NOSIGNAL | MSG_DONTWAIT)) > 0) {
                            buffer[nres] = '\0';
                            std::cout << std::setw(2) << events[eventIx].data.fd
                                      << " >> " << buffer;

                            if((nres = send(events[eventIx].data.fd, &buffer[0], nres, MSG_DONTWAIT)) == -1) {
                                errno = EBUSY;
                                break;
                            }
                        }
                    
                       if ((nres == -1) && (errno != EAGAIN)) {

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