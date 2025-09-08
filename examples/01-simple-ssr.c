#define CFRAME_IMPLEMENTATION

#include "../src/cframe.h"

HTTPResponse home(HTTPServer* server, HTTPRequest* data, char** uri_data, size_t uri_data_count) {

    string s = string_from("No data recieved");
    
    if (uri_data_count > 0) {
        s = string_from(uri_data[0]);
    } else { printf(LOGF "No data\n"); }

    DOMElement root = create_element(TAG_UL, "", "", 4, 
        create_element(TAG_LI, "", s.data, 0), 
        create_element(TAG_LI, "", "Hello World", 0), 
        create_element(TAG_LI, "", "Hello World", 0), 
        create_element(TAG_LI, "", "Hello World", 0)
    );

    string rendered = render_page_from_root(&root);
    string_drop(&s);
    return create_response(OK, MIME_TYPE_HTML, rendered.data, rendered.length);

}

HTTPResponse page_404(HTTPServer* server, HTTPRequest* data, char** uri_data, size_t uri_data_count) {

    return create_response(CLIENT_ERROR + 4, MIME_TYPE_HTML, "404, page not found", 19);

}

int main(void) {

    HTTPServer server = create_server(8080, 1024, 2, new_route("/[id]", &home, true), new_route("/", &home, false));
    server.route_404 = new_route(NULL, &page_404, false);
    start_server(&server);

    return 0;

}

//<div ><a >Hello World</a><a >Hello World</a><a >Hello World</a><a >Hello World</a></div>