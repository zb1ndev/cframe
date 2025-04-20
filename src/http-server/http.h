#if !defined(HTTP_H)
#define HTTP_H

    #include "../common.h"

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


#endif // HTTP_H