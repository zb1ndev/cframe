#if !defined(COMMON_H)
#define COMMON_H

    #include <stdio.h>
    #include "zstring.h" // zstring.h : Version 1.2.3

    #define BUFFER_SIZE 1024 * 4 // 4 Kilobytes

    #define CFRAME_MIME_DEFAULT         "application/octet-stream"
    #define CFRAME_MIME_HTML            "text/html"
    #define CFRAME_MIME_JAVA_SCRIPT     "application/javascript"
    #define CFRAME_MIME_WASM            "application/wasm"
    #define CFRAME_MIME_DATA            "application/octet-stream"

    /* Page Content for a ```404``` Error. */
    #define CFRAME_404_ERROR            \
        "HTTP/1.1 404 Not Found\r\n"    \
        "Content-Type: text/html\r\n"   \
        "Content-Length: 36\r\n"        \
        "Connection: close\r\n"         \
        "\r\n"                          \
        "<h1>404 Page Not Found</h1>"

    typedef struct HTTPResponse {
            
        int response_code;
        const char* content_type;
        const char* connection;
        String data;

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

    #if defined(_WIN32)

        #include <winsock2.h>
        #include <ws2tcpip.h> 
        #include <windows.h>

        #define perror(msg) fprintf(stderr, "%s : %d\n", msg, WSAGetLastError());
        
        typedef SOCKET Socket; 

        typedef struct HTTPHandler {

            WSADATA wsa_data;
            fd_set master_fd, read_fd;

            Socket server_socket, client_socket;
            struct sockaddr_in server_addr, client_addr;
            int addr_len;

        } HTTPHandler;

        typedef struct HTTPHandlerContext {

            HTTPHandler* handler;
            HTTPServer* server;
            Socket socket;

        } HTTPHandlerContext;

    #endif // _WIN32

    #if defined(unix)

        #include <pthread.h>
        #include <unistd.h>
        #include <netinet/in.h>  
        #include <arpa/inet.h>    

        #include <sys/types.h>
        #include <sys/wait.h>
        #include <fcntl.h>
        
        typedef int Socket;
        
        typedef struct HTTPHandler {

            

        } HTTPHandler;

        typedef struct HTTPHandlerContext {

            HTTPHandler* handler;
            HTTPServer* server;
            Socket socket;

        } HTTPHandlerContext;

    #endif // unix

#endif // COMMON_H