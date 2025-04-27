#include "./include/handler.h"

#if defined(_WIN32)

    void handle_http_request(void* context) {

        char buffer[BUFFER_SIZE];
        HTTPHandlerContext* handler_context = (HTTPHandlerContext*)context;
        int bytes_received = read_socket(handler_context->socket, buffer, BUFFER_SIZE - 1);
       
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
                String result = string_from_format(
                    "HTTP/1.1 %d OK\r\n"
                    "Content-Type: %s\r\n"
                    "Content-Length: %u\r\n"
                    "Connection: %s\r\n"
                    "\r\n"
                    "%s",
                    response.response_code,
                    response.content_type,
                    response.data.length,
                    response.connection,
                    response.data.content
                );

                write_socket(handler_context->socket, result.content, result.length);
                responeded = 1;

            }
            
        }
        
        if (responeded == 0) {   
            write_socket(handler_context->socket, CFRAME_404_ERROR, strlen(CFRAME_404_ERROR));
            FD_CLR(handler_context->socket, &handler_context->handler->master_fd);
            closesocket(handler_context->socket);
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

#endif // _WIN32

#if defined(unix)

    void handle_http_request(void* context) {

        

    }

    int run_handler(HTTPServer* server, HTTPHandler* handler) {

        int epoll_fd = epoll_create1(0);
        if (epoll_fd == -1) {
            perror("Failed to create epoll instance...");
            return -1;
        }

        struct epoll_event event, events[MAX_EVENTS];
        event.data.fd = handler->server_socket;
        event.events = EPOLLIN | EPOLLET;
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, handler->server_socket, &event);
        
        printf("Server listening on port %d\n", server->port);

    }

#endif // unix