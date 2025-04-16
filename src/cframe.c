#if !defined(Z_STRING_IMPLEMENTATION)
    #define Z_STRING_IMPLEMENTATION
#endif // Z_STRING_IMPLEMENTATION

#include "cframe.h"

#pragma region PLATFORM AGNOSTIC

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

    int handle_http_request(HTTPServer* server, ClientSocket client_socket) {

        #define BUFFER_SIZE 1024 * 4 // 4 Kilobytes
        char buffer[BUFFER_SIZE] = {0};
        int bytes_recieved = read_socket(client_socket, buffer, BUFFER_SIZE - 1);

        if (bytes_recieved <= 0) {
            close_socket(client_socket);
            return 1;
        }
    
        buffer[bytes_recieved] = '\0';

        char* uri = strtok(buffer, " ");
        uri = strtok(NULL, " ");

        int responeded = 0;
        for (size_t r = 0; r < server->routes_length; r++) {
            
            if (strcmp(uri, server->routes[r].address) == 0) {

                HTTPResponse response = server->routes[r].responder(NULL);
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

                write_socket(client_socket, result.content, result.length);
                responeded = 1;

            }
            
        }
        
        if (responeded == 0)
            write_socket(client_socket, CFRAME_404_ERROR, strlen(CFRAME_404_ERROR));
        close_socket(client_socket);

        return 0;

    }

    void close_http_server(HTTPServer* server) {
        free(server->routes);
    }

#pragma endregion
#pragma region WINDOWS

    #if defined(_WIN32)

        int start_http_server(HTTPServer* server) {

            WSADATA wsa_data;
            ClientSocket server_socket, client_socket;
            struct sockaddr_in server_addr, client_addr;
            int client_len = sizeof(client_addr);

            if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
                perror("Failed to start WSA...");
                return 1;
            }
        
            server_socket = socket(AF_INET, SOCK_STREAM, 0);
            if (server_socket == INVALID_SOCKET) {
                perror("Failed to open socket...");
                WSACleanup();
                return 1;
            }
        
            memset(&server_addr, 0, sizeof(server_addr));
        
            server_addr.sin_family = AF_INET;
            if (InetPton(AF_INET, server->address, &server_addr.sin_addr.s_addr) <= 0) {
                perror("Failed to convert address to network-byte-order...");
                return -1;
            }
            server_addr.sin_port = htons(server->port);
        
            if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
                perror("Failed to bind socket...");
                close_socket(server_socket);
                WSACleanup();
                return 1;
            }
        
            if (listen(server_socket, 5) == SOCKET_ERROR) {
                perror("Failed to listen to socket...");
                close_socket(server_socket);
                WSACleanup();
                return 1;
            }

            while (1) {

                client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);

                if (client_socket == INVALID_SOCKET) {
                    perror("Failed to accept connection...");
                    continue;
                }
        
                handle_http_request(server, client_socket);
        
            }

            close_socket(server_socket);
            WSACleanup();

            return 0;

        }

        int start_http_server_with_handler(HTTPServer* server, void(*error_handler)(int)) {

            WSADATA wsa_data;
            ClientSocket server_socket, client_socket;
            struct sockaddr_in server_addr, client_addr;
            int client_len = sizeof(client_addr);

            if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
                error_handler(CFRAME_ERROR_START);
                return 1;
            }
        
            server_socket = socket(AF_INET, SOCK_STREAM, 0);
            if (server_socket == INVALID_SOCKET) {
                error_handler(CFRAME_ERROR_OPEN_SOCKET);
                WSACleanup();
                return 1;
            }
        
            memset(&server_addr, 0, sizeof(server_addr));
        
            server_addr.sin_family = AF_INET;
            if (InetPton(AF_INET, server->address, &server_addr.sin_addr.s_addr) <= 0) {
                error_handler(CFRAME_ERROR_ADDR_TO_NBO);
                return 1;
            }
            server_addr.sin_port = htons(server->port);
        
            if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
                error_handler(CFRAME_ERROR_BIND_SOCKET);
                close_socket(server_socket);
                WSACleanup();
                return 1;
            }
        
            if (listen(server_socket, 5) == SOCKET_ERROR) {
                error_handler(CFRAME_ERROR_LISTEN_SOCKET);
                close_socket(server_socket);
                WSACleanup();
                return 1;
            }

            while (1) {

                client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
                
                if (client_socket == INVALID_SOCKET) {
                    error_handler(CFRAME_ERROR_INVALID_SOCKET);
                    continue;
                }
        
                handle_http_request(server, client_socket);
        
            }

            close_socket(server_socket);
            WSACleanup();

            return 0;

        }

        NodeInstance spawn_node_instance(const char* file) {

            NodeInstance result;
            result.attributes = (SECURITY_ATTRIBUTES){ sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };

            if (!CreatePipe(&result.child_read_stdout, &result.child_write_stdout, &result.attributes, 0)) {
                perror("Failed to create stdout pipe.");
                return (NodeInstance){ 0 };
            }
            SetHandleInformation(result.child_read_stdout, HANDLE_FLAG_INHERIT, 0);

            if (!CreatePipe(&result.child_read_stdin, &result.child_write_stdin, &result.attributes, 0)) {
                perror("Failed to create stdout pipe.");
                return (NodeInstance){ 0 };
            }
            SetHandleInformation(result.child_read_stdin, HANDLE_FLAG_INHERIT, 0);

            ZeroMemory(&result.process_info, sizeof(PROCESS_INFORMATION));
            ZeroMemory(&result.start_info, sizeof(STARTUPINFO));

            result.start_info.cb = sizeof(STARTUPINFO);
            result.start_info.hStdError = result.child_write_stdout;
            result.start_info.hStdOutput = result.child_write_stdout;
            result.start_info.hStdInput = result.child_read_stdin;
            result.start_info.dwFlags |= STARTF_USESTDHANDLES;

            String cmd = string_from_format("%s %s", "node.exe", file);
            if (!CreateProcess(NULL, cmd.content, NULL, NULL, TRUE, 0, NULL, NULL, &result.start_info, &result.process_info)) {
                perror("Failed to create node instance.");
                return (NodeInstance){ 0 };
            }

            CloseHandle(result.child_write_stdout);
            CloseHandle(result.child_read_stdin);

            return result;

        }
        
        void destroy_node_instance(NodeInstance* instance) {

            if (instance->child_read_stdout == NULL) CloseHandle(instance->child_read_stdout);
            if (instance->child_write_stdin == NULL) CloseHandle(instance->child_write_stdin);

            WaitForSingleObject(instance->process_info.hProcess, INFINITE);

            CloseHandle(instance->process_info.hProcess);
            CloseHandle(instance->process_info.hThread);

        }

        String read_node_instance(NodeInstance* instance) {

            String result = string_from("");
            char buffer[128];
            DWORD bytes_read;
            while(ReadFile(instance->child_read_stdout, buffer, sizeof(buffer) - 1, &bytes_read, NULL) && bytes_read > 0) {
                string_append(&result, buffer);
            }
            return result;

        }

        int write_node_instance(NodeInstance* instance, String input) {

            DWORD bytes_written;
            WriteFile(instance->child_write_stdin, input.content, (DWORD)input.length, &bytes_written, NULL);
            CloseHandle(instance->child_write_stdin);
            instance->child_write_stdin = NULL;

            return 0;

        }

    #endif // _WIN32

#pragma endregion
#pragma region UNIX

    #if defined(unix)

        int start_http_server(HTTPServer* server) {
                
            int server_fd, client_socket;
            struct sockaddr_in address;
            socklen_t addrlen = sizeof(address);

            server_fd = socket(AF_INET, SOCK_STREAM, 0);
            if (server_fd == 0) {
                perror("Failed to open socket...");
                return 1;
            }

            address.sin_family = AF_INET;
            address.sin_port = htons(server->port);
            if (inet_pton(AF_INET, server->address, &address.sin_addr) <= 0) {
                perror("Failed to convert address to network-byte-order...");
                return 1;
            }

            if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
                perror("Failed to bind socket...");
                close(server_fd);
                return 1;
            }
        
            if (listen(server_fd, 5) < 0) {
                perror("Failed to listen to socket...");
                close(server_fd);
                return 1;
            }

            while (1) {

                client_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
                if (client_socket < 0) {
                    perror("Failed to accept connection...");
                    continue;
                }
                handle_http_request(server, client_socket);

            }
        
            close(server_fd);
            return 0;

        }

        int start_http_server_with_handler(HTTPServer* server, void(*error_handler)(int)) {
            
            int server_fd, client_socket;
            struct sockaddr_in address;
            socklen_t addrlen = sizeof(address);

            server_fd = socket(AF_INET, SOCK_STREAM, 0);
            if (server_fd == 0) {
                error_handler(CFRAME_ERROR_OPEN_SOCKET);
                return 1;
            }

            address.sin_family = AF_INET;
            address.sin_port = htons(server->port);
            if (inet_pton(AF_INET, server->address, &address.sin_addr) <= 0) {
                error_handler(CFRAME_ERROR_ADDR_TO_NBO);
                return 1;
            }

            if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
                error_handler(CFRAME_ERROR_BIND_SOCKET);
                close(server_fd);
                return 1;
            }
        
            if (listen(server_fd, 5) < 0) {
                error_handler(CFRAME_ERROR_LISTEN_SOCKET);
                close(server_fd);
                return 1;
            }

            while (1) {

                client_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
                if (client_socket < 0) {
                    error_handler(CFRAME_ERROR_INVALID_SOCKET);
                    continue;
                }
                handle_http_request(server, client_socket);

            }
        
            close(server_fd);
            return 0;
        }

        NodeInstance spawn_node_instance(const char* file) { 

            NodeInstance result = {0};
            
            if (pipe(result.child_stdout_pipe) == -1) {
                perror("Failed to create stdout pipe...");
                return (NodeInstance){0};
            }

            if (pipe(result.child_stdin_pipe) == -1) {
                perror("Failed to create stdin pipe...");
                return (NodeInstance){0};
            }

            pid_t pid = fork();
            if (pid < 0) {
                perror("Failed to create node instance...");
                return (NodeInstance){0};
            }

            if (pid == 0) {

                dup2(result.child_stdout_pipe[1], STDOUT_FILENO);
                dup2(result.child_stdin_pipe[0], STDIN_FILENO);
                dup2(result.child_stdout_pipe[1], STDERR_FILENO); 

                execlp("node", "node", file, (char*)NULL);
                perror("Failed to execute node instance...");
                exit(1);

            }

            result.available = 1;
            result.pid = pid;

            close(result.child_stdout_pipe[1]);
            close(result.child_stdin_pipe[0]);
            return result;

        }
    
        void destroy_node_instance(NodeInstance* instance) {

            if (!instance || !instance->available) return;
            if (instance->child_stdout_pipe[0] >= 0)
                close(instance->child_stdout_pipe[0]);
            if (instance->child_stdin_pipe[1] >= 0)
                close(instance->child_stdin_pipe[1]);
        
            int status = 0;
            waitpid(instance->pid, &status, 0);
            instance->available = 0;

        }

        String read_node_instance(NodeInstance* instance) {

            String result = string_from("");
            char buffer[128];
            ssize_t bytes_read;
        
            while ((bytes_read = read(instance->child_stdout_pipe[0], buffer, sizeof(buffer) - 1)) > 0) {
                buffer[bytes_read] = '\0';
                string_append(&result, buffer);
            }
        
            return result;

        }

        int write_node_instance(NodeInstance* instance, String input) {
            
            ssize_t bytes_written = write(instance->child_stdin_pipe[1], input.content, input.length);
        
            close(instance->child_stdin_pipe[1]);
            instance->child_stdin_pipe[1] = -1;
        
            return (bytes_written == input.length) ? 0 : -1;

        }

    #endif // unix

#pragma endregion