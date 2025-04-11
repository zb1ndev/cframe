/* cframe - v1.0.0 - MIT License - https://github.com/zb1ndev/zansi.h 

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

    WARNING : Depends on https://github.com/zb1ndev/zstring.h | Minimum Version : 1.2.2

*/

#if !defined(CFRAME_H)
#define CFRAME_H

    #pragma region INCLUDES

        #include <stdio.h>
        #include <stdlib.h>
        #include <stdbool.h>

        #include "../deps/zstring.h" // zstring.h : Version 1.2.2

        #if defined(_WIN32) 

            #include <winsock2.h>
            #include <ws2tcpip.h> 
            #include <windows.h>

        #elif defined(unix)

            // UNIX includes

        #endif

    #pragma endregion
    #pragma region TYPES

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

        typedef struct NodeInstance {

            char available;

            STARTUPINFO start_info;
            PROCESS_INFORMATION process_info;
            SECURITY_ATTRIBUTES attributes;

            HANDLE child_read_stdout, child_write_stdout; 
            HANDLE child_read_stdin, child_write_stdin;

        } NodeInstance;

    #pragma endregion
    #pragma region ERRORS

        #define CFRAME_ERROR_START 0
        #define CFRAME_ERROR_OPEN_SOCKET 1
        
        /* Occurs when cframe fails to convert address to network-byte-order. */
        #define CFRAME_ERROR_ADDR_TO_NBO 2

        #define CFRAME_ERROR_BIND_SOCKET 3
        #define CFRAME_ERROR_LISTEN_SOCKET 4
        #define CFRAME_ERROR_INVALID_SOCKET 5

        /* Page Content for a ```404 : Page Not Found``` Error. */
        #define CFRAME_404_ERROR "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n<h1> 404 Page Not Found</h1>"

    #pragma endregion

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

    /** A function that handles requests for the HTTP Server.
     * @param server The server you want to start.
     * @returns Whether the function has succeded ```0 = success```. 
     */
    int handle_http_request(HTTPServer* server, SOCKET client_socket);

    /** A function that starts the specified HTTP Server.
     * @param server The server you want to start.
     * @returns Whether the function has succeded ```0 = success```. 
     */
    int start_http_server(HTTPServer* server);

    /** A function that starts the specified HTTP Server and runs ```error_handler``` when an error occurs.
     * @param server The server you want to start.
     * @param error_handler A pointer to a function that the user defines to handle errors themselves.
     * @returns Whether the function has succeded ```0 = success```. 
     */
    int start_http_server_with_handler(HTTPServer* server, void(*error_handler)(int));
    
    /** A function that closes the specified HTTP Server.
     * @param server The server you want to close.
     */
    void close_http_server(HTTPServer* server);

    /** A function that Spawns a node instance with piped ```stdin``` and ```stdout```.
     * @param file The file you want node to run.
     * @returns A Node Instance.
     */
    NodeInstance spawn_node_instance(const char* file);

    /** A function that takes in a ```NodeInstance``` and destroys it.
     * @param instance A pointer to a ```NodeInstance```.
     */
    void destroy_node_instance(NodeInstance* instance);

    /** A function that takes in a ```NodeInstance``` and returns the contents of its ```stdout``` pipe.
     * @param instance A pointer to a ```NodeInstance```.
     * @returns The contents of its ```stdout``` pipe.
     */
    String read_node_instance(NodeInstance* instance);

     /** A function that takes in a ```NodeInstance``` and writes to its ```stdin``` pipe.
     * @param instance A pointer to a ```NodeInstance```.
     * @param input The content you want to write to ```instance```'s ```stdin``` pipe.
     * @returns Whether the function has succeded ```0 = success```.
     */
    int write_node_instance(NodeInstance* instance, String input);

#endif // CFRAME_H