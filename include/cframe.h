/* cframe - v1.3.1 - MIT License - https://github.com/zb1ndev/zansi.h 

    MIT License
    Copyright (c) 2025 Joel Zbinden

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

    WARNING : Depends on https://github.com/zb1ndev/zstring.h | Minimum Version : 1.3.1

*/
#if !defined(CFRAME_H)
#define CFRAME_H

    /**
     * zstring.h includes the following dependencies -
     *   - stdio.h          // printf @ string_from_format | error checking
     *   - stdlib.h         // realloc
     *   - string.h         // memcpy
     *   - stdarg.h         // va_args
     *   - sys/types.h      // ssize_t
     */
    #include "zstring.h" // zstring.h : Version 1.2.3

    #if defined(_WIN32)

        #include <winsock2.h>
        #include <ws2tcpip.h> 
        #include <windows.h>

        typedef SOCKET Socket;

        #define is_socket_invalid(socket) (socket == INVALID_SOCKET)
        #define close_socket(socket) closesocket(socket)
        #define write_socket(socket, content, length) send(socket, content, length, 0)
        #define read_socket(socket, buffer, length) recv(socket, buffer, length, 0)

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

    #define cframe_log_info(msg, ...)       printf("[INFO] " msg "\n", __VA_ARGS__)
    #define cframe_log_error(msg, ...)      printf("[ERROR] " msg "\n", __VA_ARGS__)
    #define cframe_log_warning(msg, ...)    printf("[WARNING] " msg "\n", __VA_ARGS__)

    #define CFRAME_DEFAULT_BUFFER_SIZE 1024 * 4 // 4 Kilobytes

    #define CFRAME_MIME_DEFAULT         "application/octet-stream"
    #define CFRAME_MIME_HTML            "text/html"
    #define CFRAME_MIME_JAVA_SCRIPT     "application/javascript"
    #define CFRAME_MIME_WASM            "application/wasm"
    #define CFRAME_MIME_DATA            "application/octet-stream"

    #define CFRAME_CLOSE                "close"
    #define CFRAME_KEEP_ALIVE           "keep-alive"

    // Page Content for a ```404``` Error
    #define CFRAME_404_ERROR            \
        "HTTP/1.1 404 Not Found\r\n"    \
        "Content-Type: text/html\r\n"   \
        "Content-Length: 27\r\n"        \
        "Connection: close\r\n"         \
        "\r\n"                          \
        "<h1>404 Page Not Found</h1>"

    typedef struct HTTPResponse {
            
        int response_code;
        const char* response_status;

        const char* content_type;
        const char* connection;
        
        size_t data_length;
        unsigned char* data;

    } HTTPResponse;

    typedef struct Route {
            
        const char* address;    
        HTTPResponse (*responder)(void*);
    
    } Route;

    typedef struct HTTPServer {
        
        const char* address;
        int port;

        Route* routes;
        size_t routes_length;

    } HTTPServer;

    typedef struct HTTPHandler {

        #if defined(_WIN32)
            
            WSADATA wsa_data;
            fd_set master_fd, read_fd;

        #endif // _WIN32

        Socket server_socket, client_socket;
        struct sockaddr_in server_addr, client_addr;
        int addr_len;

    } HTTPHandler;

    typedef struct HTTPHandlerContext {

        HTTPHandler* handler;
        HTTPServer* server;
        Socket socket;

        void* data;

    } HTTPHandlerContext;

    /** A function that opens a socket using the specified info.
     * @param handler A reference to the handler in which the socket will open.
     * @param af The address family.
     * @param type The socket type.
     * @param protocol The protocol to use with the given address family and type. 
     * @returns The opened socket. 
     */
    Socket open_socket(HTTPHandler* handler, int af, int type, int protocol);

    /** A function that initializes a Route.
     * @param address The address you want to tie the ```responder``` to.
     * @param responder A pointer to a function that will handle the response of the Route. 
     * @returns A new route with the provided information.
     */
    Route new_route(const char* address, HTTPResponse (*responder)(void*));    

    /** A function that initializes a HTTP Server at ```address```:```port```.
     * @param address A String containing the address you want the server to listen to.
     * @param port An integer containing the port you want the server to listen to.
     * @param n The number of routes you want the server to service.
     * @param __VA_ARGS__ A list of routes you want the server to service.
     * @returns A new HTTP Server containing the provided information.
     */
    HTTPServer new_http_server(const char* address, int port, size_t n, ...);

    /** A function that starts the specified HTTP Server.
     * @param server The server you want to start.
     * @returns Whether the function has succeded ```0 = success```. 
     */
    int start_http_server(HTTPServer* server);

    /** A function that closes the specified HTTP Server.
     * @param server The server you want to close.
     */
    void close_http_server(HTTPServer* server);

    /** A function that handles http requests.
     * @param context The http request context.
     */
    void handle_http_request(void* context);

    /** A function that runs the handler.
     * @param handler The handler you want to handle http requests with.
     * @returns Whether the function has succeded ```0 = success```.
     */
    int run_handler(HTTPServer* server, HTTPHandler* handler);

// #define CFRAME_IMPLEMENTATION // Remove for RELEASE
#if defined(CFRAME_IMPLEMENTATION)

    Socket open_socket(HTTPHandler* handler, int af, int type, int protocol) {
        
        #if defined(_WIN32)
            if (WSAStartup(MAKEWORD(2, 2), &handler->wsa_data) != 0) {
                cframe_log_error("%s", "Failed to start WSA.");
                return 0;
            }
        #endif // _WIN32
        return socket(AF_INET, SOCK_STREAM, 0);

    }

    Route new_route(const char* address, HTTPResponse (*responder)(void*)) {
        return (Route){address, responder};
    }

    HTTPServer new_http_server(const char* address, int port, size_t n, ...) {

        Route* routes = (Route*)malloc(sizeof(Route) * n);
        if (routes == NULL) {
            cframe_log_error("%s", "Failed to allocate space for routes.");
            exit(1);
        }             

        va_list args;
        va_start(args, n);
        for (size_t r = 0; r < n; r++)
            routes[r] = va_arg(args, Route);
        va_end(args);

        return (HTTPServer){address, port, routes, n};

    }

    int start_http_server(HTTPServer* server) {

        HTTPHandler handler;
        handler.addr_len = sizeof(handler.client_addr);

        handler.server_socket = open_socket(&handler, AF_INET, SOCK_STREAM, 0);
        if (is_socket_invalid(handler.server_socket)) {
            perror("Failed to open socket...");
            return 1;
        }

        set_server_addr(AF_INET, server->address, server->port, handler.server_addr);

        if (bind(handler.server_socket, (struct sockaddr *)&handler.server_addr, sizeof(handler.server_addr)) < 0) {
            perror("Failed to bind socket...");
            close_socket(handler.server_socket);
            return 1;
        }
        
        if (listen(handler.server_socket, 5) < 0) {
            perror("Failed to listen to socket...");
            close_socket(handler.server_socket);
            return 1;
        }

        printf("[\e[0;30mPROCESS\e[0m] HTTP Server running on '\e[0;36m%s:%d\e[0m'\n", server->address, server->port);

        if (run_handler(server, &handler) < 0) {
            perror("Something went wrong when handling a request...");
            close_socket(handler.server_socket);
            return 1;
        }

        close_socket(handler.server_socket);
        return 0;

    }

    void close_http_server(HTTPServer* server) {
        
        free(server->routes);

        #if defined(_WIN32)
            WSACleanup();
        #endif // _WIN32

    }

    void handle_http_request(void* context) {

        char buffer[CFRAME_DEFAULT_BUFFER_SIZE];
        HTTPHandlerContext* handler_context = (HTTPHandlerContext*)context;
        int bytes_received = read_socket(handler_context->socket, buffer, CFRAME_DEFAULT_BUFFER_SIZE - 1);
       
        if (bytes_received <= 0) {
            printf("Connection closed or recv failed\n");
            FD_CLR(handler_context->socket, &handler_context->handler->master_fd);
            closesocket(handler_context->socket);
            return;
        }
        buffer[bytes_received] = '\0';
        
        char* uri = strtok(buffer, " ");
        uri = strtok(NULL, " ");

        int responeded = 0;
        for (size_t r = 0; r < handler_context->server->routes_length; r++) {
            
            if (strcmp(uri, handler_context->server->routes[r].address) == 0) {

                HTTPResponse response = handler_context->server->routes[r].responder(handler_context);
                String head = string_from_format (

                    "HTTP/1.1 %d %s\r\n"
                    "Content-Type: %s\r\n"
                    "Content-Length: %u\r\n"
                    "Connection: %s\r\n"
                    "\r\n",

                    response.response_code,
                    response.response_status,
                    response.content_type,
                    response.data_length,
                    response.connection

                );

                size_t result_length = (response.data_length + head.length);
                char* result = (char*)malloc(result_length * sizeof(unsigned char));
                if (result == NULL)
                    break;

                memcpy(result, head.content, head.length);
                memcpy(result + head.length, response.data, response.data_length);

                write_socket(handler_context->socket, result, result_length);
                responeded = 1;

                free(result);
                free(response.data);
                string_drop(&head);

            }
            
        }
        
        if (responeded == 0) {   
            write_socket(handler_context->socket, CFRAME_404_ERROR, strlen(CFRAME_404_ERROR));
        }

        FD_CLR(handler_context->socket, &handler_context->handler->master_fd);
        closesocket(handler_context->socket);

    }

    int run_handler(HTTPServer* server, HTTPHandler* handler) {

        FD_ZERO(&handler->master_fd);
        FD_SET(handler->server_socket, &handler->master_fd);
        int max_fd = handler->server_socket;

        while (1) {

            handler->read_fd = handler->master_fd;
            if (select(max_fd + 1, &handler->read_fd, NULL, NULL, NULL) == SOCKET_ERROR) {
                perror("Select failed...");
                return -1;
            }
            
            for (int s = 0; s <= max_fd; s++) {
                if (FD_ISSET(s, &handler->read_fd)) {
                    if (s == handler->server_socket) {

                        handler->client_socket = accept(handler->server_socket, (struct sockaddr*)&handler->client_addr, &handler->addr_len);
                        if (handler->client_socket == INVALID_SOCKET) {
                            perror("Failed to accept connection...");
                            continue;
                        }

                        FD_SET(handler->client_socket, &handler->master_fd);
                        if (handler->client_socket > max_fd) {
                            max_fd = handler->client_socket;
                        }

                    } else { 

                        HTTPHandlerContext context = (HTTPHandlerContext){handler, server, s, NULL};
                        handle_http_request(&context); 
                    
                    }
                }
            }

        }

        return 0;

    }

#endif // CFRAME_IMPLEMENTATION
#endif // CFRAME_H