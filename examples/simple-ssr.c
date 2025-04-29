#include "../src/include/cframe.h"

HTTPResponse home(void* data) {

    HTTPHandlerContext context = *(HTTPHandlerContext*)data;

    return (HTTPResponse) {
        200, CFRAME_MIME_HTML, CFRAME_CLOSE,
        string_from_format("This server is running on %s:%d", context.server->address, context.server->port)
    };

}

int main(void) {

    HTTPServer server = new_http_server("127.0.0.1", 8080, 1, new_route("/", &home));
    start_http_server(&server);
    close_http_server(&server);

    return 0;

}