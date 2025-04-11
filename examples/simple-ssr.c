#include <stdio.h>
#include "../src/cframe.h"

HTTPResponse home(void* data) {

    return (HTTPResponse) {
        200, "text/html", "close", 
        string_from("<h1>This is a home page <a href=\"/test\">test</a></h1>")
    };
 
}

HTTPResponse test(void* data) {

    NodeInstance n1 = spawn_node_instance("./examples/main.js");
    String result = read_node_instance(&n1);
    destroy_node_instance(&n1);

    return (HTTPResponse) {
        200, "text/html", "close", 
        result
    };
 
}

int main() {

    HTTPServer server = new_http_server(
        "127.0.0.1", 8080, 2, 
        new_route("/", &home),
        new_route("/test", &test)
    );
    
    start_http_server(&server);
    close_http_server(&server);

    return 0;


}