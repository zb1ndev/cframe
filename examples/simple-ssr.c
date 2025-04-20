#include "../src/cframe.h"

HTTPResponse home(void* data) {

    return (HTTPResponse) {
        200, CFRAME_MIME_HTML, "close",
        string_from("Hello World")
    };

}

int main(void) {

    HTTPServer server = new_http_server("127.0.0.1", 8080, 1, new_route("/", &home));
    start_http_server(&server);
    close_http_server(&server);

    return 0;

}