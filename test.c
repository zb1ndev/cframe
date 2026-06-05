#define CFRAME_IMPLEMENTATION
#include "cframe.h"

http_response_t home(http_server_t* server, http_request_t* request, string_t* uri_data, size_t uri_data_count) {

    return http_create_response(200, MIME_TYPE_HTML, "<a>hello world</a>", 18);

}

int main(void) {

    http_route_t route_404 = (http_route_t) {
        .uri = NULL,
        .handler = &home
    };

    http_route_t* routes = calloc(1, sizeof(http_route_t));
    *routes = (http_route_t) {
        .dynamic = false,
        .uri = "/",
        .handler = &home
    };

    http_server_t server = (http_server_t) {
        .port = 4040,
        .route_404 = route_404,
        .routes = routes,
        .route_count = 1,
        .max_events = 10,
        .workers = 4,
        .running = true,
    };

    int result = http_server_start(&server);

    printf("Result %d\n", result);
    return 0;

}