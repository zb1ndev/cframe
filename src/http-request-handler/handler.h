#if !defined(HANDLER_H)
#define HANDLER_H

    #include "../common.h"

    #if defined(_WIN32)
        
        #define is_socket_invalid(socket) (socket == INVALID_SOCKET ? 0 : 1)

        Socket open_socket(HTTPHandler* handler, int af, int type, int protocol);
        #define write_socket(socket, content, length) send(socket, content, length, 0)
        #define read_socket(socket, buffer, length) recv(socket, buffer, length, 0)

        #define perror(msg) fprintf(stderr, "%s : %d\n", msg, WSAGetLastError());

        #define set_server_addr(flag, new_addr, new_port, addr)                             \
            memset(&addr, 0, sizeof(addr));                                                 \
            addr.sin_family = flag;                                                         \
            if (InetPton(flag, new_addr, &addr.sin_addr.s_addr) <= 0) {                     \
                perror("Failed to convert address to network-byte-order...");               \
                WSACleanup();                                                               \
                return -1;                                                                  \
            }                                                                               \
            addr.sin_port = htons(new_port);

    #endif // _WIN32

    #if defined(unix)

        #define is_socket_invalid(socket) (socket < 0 ? 0 : 1)

        Socket open_socket(HTTPHandler* handler, int af, int type, int protocol);
        #define close_socket(socket) close(socket)
        #define write_socket(socket, content, length) write(socket, content, length)
        #define read_socket(socket, buffer, length) read(socket, buffer, length)

        #define set_server_addr(flag, val, addr)                                    \
            memset(&addr, 0, sizeof(addr));                                         \
            addr.sin_family = flag;                                                 \
            if (inet_pton(flag, new_addr, &addr.sin_addr) <= 0) {                   \
                perror("Failed to convert address to network-byte-order...");       \
                return 1;                                                           \
            }                                                                       \
            addr.sin_port = htons(new_port);

    #endif // unix

    /** A function that handles http requests.
     * @param context The http request context.
     */
    void handle_http_request(void* context);

    /** A function that runs the handler.
     * @param handler The handler you want to handle http requests with.
     * @returns Whether the function has succeded ```0 = success```.
     */
    int run_handler(HTTPServer* server, HTTPHandler* handler);

#endif // HANDLER_H