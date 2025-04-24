#include "http.h"
#include "../http-request-handler/handler.h" // run_handler

#pragma region UTILITIES

    Socket open_socket(HTTPHandler* handler, int af, int type, int protocol) {
        
        #if defined(_WIN32)

            if (WSAStartup(MAKEWORD(2, 2), &handler->wsa_data) != 0) {
                perror("Failed to start WSA...");
                return 0;
            }

        #endif

        return socket(AF_INET, SOCK_STREAM, 0);

    }

#pragma endregion
#pragma region IMPLEMENTATION

    Route new_route(const char* address, HTTPResponse (*responder)(void*)) {
        return (Route){address, responder};
    }

    HTTPServer new_http_server(const char* address, int port, size_t n, ...) {

        Route* routes = malloc(sizeof(Route) * n);
        if (routes == NULL) {
            perror("Failed to allocate space for routes.");
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

        printf("HTTP Server running on %s:%d\n", server->address, server->port);

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

#pragma endregion