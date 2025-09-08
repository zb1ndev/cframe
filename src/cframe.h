    
    /*                                              MIT License                                      */
    /*                                   Copyright (c) 2025 Joel Zbinden                             */

    /*     Please note this library was built for personal use on Oracle ARM-Based Linux Servers     */
    /*      If the lack of "namespacing" or cross-platform bothers you, figure it out yourself.      */

#if !defined(CFRAME_H)
#define CFRAME_H

    #if defined(CFRAME_IMPLEMENTATION)
        #define CSTRING_IMPLEMENTATION
    #endif // CFRAME_IMPLEMENTATION

    #include <stdbool.h>
    #include "../deps/cstring.h"

    #include <fcntl.h>
    #include <errno.h>
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <sys/epoll.h>
    #include <sys/socket.h>

    #define MIME_TYPE_HTML              "text/html"
    #define MIME_TYPE_WASM              "application/wasm"
    #define MIME_TYPE_JAVA_SCRIPT       "application/javascript"
    #define MIME_TYPE_DATA              "application/octet-stream"
    #define MIME_TYPE_DEFAULT           "application/octet-stream"
    
    #define empty_http_response()       (HTTPResponse){SERVER_ERROR, NULL, 0}
    #define empty_dom_element()         (DOMElement){0,NULL,NULL,empty_string(),NULL,0}
    
    typedef unsigned char               byte;
        
    typedef struct Route                Route;
    typedef struct DOMElement           DOMElement;
    typedef struct HTTPServer           HTTPServer;
    typedef struct HTTPRequest          HTTPRequest;
    typedef struct HTTPResponse         HTTPResponse;

    typedef enum ResponseCode {

        OK                  = 200,
        REDIRECTION         = 300,
        CLIENT_ERROR        = 400,
        SERVER_ERROR        = 500

    } ResponseCode;

    typedef                 HTTPResponse                (*RouteHandler)(HTTPServer* server, HTTPRequest* data, char** uri_data, size_t uri_data_count);

    typedef struct Route {

        bool                dynamic;
        char*               uri;
        RouteHandler        route_handler;

    } Route;


    typedef struct HTTPServer {

        int                 port;
        size_t              max_events;

        Route               route_404;
        Route*              routes;
        size_t              route_count;

        int                 fd;
        struct sockaddr_in  address;

        int                 epoll_fd;

    } HTTPServer;

    typedef struct HTTPRequest {

        byte*               raw_data;
        size_t              raw_data_length;
        char*               uri;

    } HTTPRequest;


    typedef struct HTTPResponse {

        ResponseCode        code;
        byte*               response;
        size_t              response_length;

    } HTTPResponse;

    HTTPServer              create_server               (int port, size_t max_events, size_t route_count, ...);
    int                     start_server                (HTTPServer* server);

    int                     poll_server_events          (HTTPServer* server);
    int                     handle_request              (HTTPServer* server, HTTPRequest request, int client);
    int                     accept_connection           (HTTPServer* server, struct epoll_event* event);

    char**                  chop_uri                    (char* src, size_t* slices);
    void                    free_chopped_uri            (char** src, size_t count);
    Route                   new_route                   (char* uri, RouteHandler route_handler, bool dynamic);

    HTTPResponse            create_response             (ResponseCode code, const char* mime, byte* data, size_t data_length);

    typedef enum HTMLTag {

        TAG_A, TAG_ABBR, TAG_ADDRESS, TAG_AREA, TAG_ARTICLE, TAG_ASIDE, TAG_AUDIO, TAG_B,
        TAG_BASE, TAG_BDI, TAG_BDO, TAG_BLOCKQUOTE, TAG_BODY, TAG_BR, TAG_BUTTON, TAG_CANVAS,
        TAG_CAPTION, TAG_CITE, TAG_CODE, TAG_COL, TAG_COLGROUP, TAG_DATA, TAG_DATALIST, TAG_DD,
        TAG_DEL, TAG_DETAILS, TAG_DFN, TAG_DIALOG, TAG_DIV, TAG_DL, TAG_DT, TAG_EM, TAG_FIELDSET,
        TAG_FIGCAPTION, TAG_FIGURE, TAG_FOOTER, TAG_FORM, TAG_H1, TAG_H2, TAG_H3, TAG_H4, TAG_H5, 
        TAG_H6, TAG_HEAD, TAG_HEADER, TAG_HR, TAG_HTML, TAG_I, TAG_IFRAME, TAG_IMG, TAG_INPUT, 
        TAG_INS, TAG_KBD, TAG_LABEL, TAG_LEGEND, TAG_LI, TAG_LINK, TAG_MAIN, TAG_MAP, TAG_MARK,
        TAG_MENU, TAG_META, TAG_METER, TAG_NAV, TAG_NOSCRIPT, TAG_OBJECT, TAG_OL, TAG_OPTGROUP, 
        TAG_OPTION, TAG_OUTPUT, TAG_P, TAG_PARAM, TAG_PICTURE, TAG_PRE, TAG_PROGRESS, TAG_Q, TAG_RP,
        TAG_RT, TAG_RUBY, TAG_S, TAG_SAMP, TAG_SCRIPT, TAG_SECTION, TAG_SELECT, TAG_SMALL, TAG_SOURCE,
        TAG_SPAN, TAG_STRONG, TAG_STYLE, TAG_SUB, TAG_SUMMARY, TAG_SUP, TAG_TABLE, TAG_TBODY, TAG_TD,
        TAG_TEMPLATE, TAG_TEXTAREA, TAG_TFOOT, TAG_TH, TAG_THEAD, TAG_TIME, TAG_TITLE, TAG_TR, TAG_TRACK, 
        TAG_U, TAG_UL, TAG_VAR, TAG_VIDEO, TAG_WBR
        
    } HTMLTag;

    static const char* HTMLTagNames[] = {

        "a","abbr","address","area","article","aside","audio", "b","base","bdi","bdo","blockquote",
        "body","br","button","canvas","caption","cite","code","col","colgroup","data","datalist",
        "dd","del","details","dfn","dialog","div","dl","dt","em","fieldset","figcaption","figure",
        "footer","form","h1","h2","h3","h4","h5","h6","head","header","hr","html","i","iframe",
        "img","input","ins","kbd","label","legend","li","link","main","map","mark","menu","meta",
        "meter","nav","noscript","object","ol","optgroup","option","output","p","param","picture",
        "pre","progress","q","rp","rt","ruby","s","samp","script","section","select","small",
        "source","span","strong","style","sub","summary","sup","table","tbody","td","template",
        "textarea","tfoot","th","thead","time","title","tr","track","u","ul","var","video","wbr"
    
    };

    typedef struct DOMElement {

        HTMLTag             tag;
        char*               props;
        char*               content;

        string              rendered;

        DOMElement*         children;
        size_t              child_count;

    } DOMElement;

    DOMElement              create_element                  (HTMLTag tag, char* props, char* content, size_t child_count, ...);
    void                    free_dom_element                (DOMElement* element);

    string                  render_page_from_root           (DOMElement* root);
    
#if defined(CFRAME_IMPLEMENTATION)

    HTTPServer create_server(int port, size_t max_events, size_t route_count, ...) {

        HTTPServer result = (HTTPServer) {
            
            .port           = port,
            .max_events     = max_events,
            .route_count    = route_count,

            .routes         = NULL
        
        };
        
        if (route_count <= 0) 
            return result;

        Route* temp = (Route*)malloc(sizeof(Route) * route_count);
        if (temp == NULL) {
            printf(ERRORF "Failed to allocate memory for routes.\n");
            return result;
        }
        result.routes = temp;

        va_list list;
        va_start(list, route_count);

        for (size_t r = 0; r < route_count; r++)
            result.routes[r] = va_arg(list, Route);
        
        va_end(list);

        return result;

    }

    int start_server(HTTPServer* server) {

        if (server == NULL) {
            printf(ERRORF "No server object provided.\n");
            return -1;
        }

        server->fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server->fd == -1) {
            printf(ERRORF "Failed to create socket.\n");
            return -1;
        }

        int opt = 1;
        setsockopt(server->fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        memset(&server->address, 0, sizeof(server->address));
        server->address.sin_family      = AF_INET;
        server->address.sin_addr.s_addr = INADDR_ANY;
        server->address.sin_port        = htons(server->port);

        if (bind(server->fd, (struct sockaddr*)&server->address, sizeof(server->address)) < 0) {
            printf(ERRORF "Failed to bind to socket.\n");
            close(server->fd);
            return -1;
        }

        if (listen(server->fd, SOMAXCONN) < 0) {
            printf(ERRORF "Faild to listen to port %d.\n", server->port);
            close(server->fd);
            return -1;
        }

        return poll_server_events(server);

    }

    int poll_server_events(HTTPServer* server) {

        if (server == NULL) {
            printf(ERRORF "No server object provided.\n");
            return -1;
        }

        int flags = fcntl(server->fd, F_GETFL, 0);
        if (fcntl(server->fd, F_SETFL, flags | O_NONBLOCK) < 0) {
            printf(ERRORF "Failed to set server socket to non-blocking.\n");
            return -1;
        }

        server->epoll_fd = epoll_create1(0);
        if (server->epoll_fd < 0) {
            printf(ERRORF "Failed to create epoll socket.\n");
            close(server->fd);
            return -1;
        }

        int connection_fd = -1;
        struct epoll_event event; 
        struct epoll_event events[server->max_events];

        event.data.fd   = server->fd;
        event.events    = EPOLLIN;

        if (epoll_ctl(server->epoll_fd, EPOLL_CTL_ADD, server->fd, &event) < 0) {
            printf(ERRORF "Failed to add server port to epoll instance.\n");
            close(server->fd);
            close(server->epoll_fd);
            return -1;
        }

        printf(LOGF "Server running on port %d...\n", server->port);

        for (;;) {

            int event_count = epoll_wait(server->epoll_fd, events, server->max_events, -1);

            for (size_t e = 0; e < event_count; e++) {

                if (events[e].data.fd == server->fd) {
                    connection_fd = accept_connection(server, &event);
                    continue;
                }

                if (connection_fd < 0)
                    continue;

                byte buffer[1024 * 8];
                
                for (;;) {

                    int bytes_read = read(events[e].data.fd, buffer, sizeof(buffer));

                    if (bytes_read == -1) {

                        if (errno == EAGAIN || errno == EWOULDBLOCK)
                            break;
                        
                        printf(ERRORF "Failed to read from client.\n");
                        close(events[e].data.fd);
                        close(server->fd);
                        close(server->epoll_fd);
                        return -1;

                    }

                    if (bytes_read == 0) {
                        close(events[e].data.fd);
                        break;
                    }
                    
                    string uri = string_from(buffer);
                    char* uri_ptr = uri.data;
                    uri.data = strtok(uri.data, " ");
                    uri.data = strtok(NULL, " ");

                    HTTPRequest request = (HTTPRequest) {
                        .raw_data = buffer,
                        .raw_data_length = bytes_read,
                        .uri = uri.data
                    };

                    if (handle_request(server, request, events[e].data.fd) < 0) {
                        printf(ERRORF "Failed to handle request.\n");
                        close(events[e].data.fd);
                        close(server->fd);
                        close(server->epoll_fd);
                        return -1;
                    }

                    free(uri_ptr);

                }

            }

        }

        close(server->fd);
        return 0;

    }

    int accept_connection(HTTPServer* server, struct epoll_event* event) {
        
        if (server == NULL) {
            printf(ERRORF "No server object provided.\n");
            return -1;
        }

        if (event == NULL) {
            printf(ERRORF "No event object provided.\n");
            return -1;
        }

        for (;;) {

            int new_connection = -1;
            struct sockaddr_in client;
            socklen_t client_len = sizeof(client);
            
            new_connection = accept(server->fd, (struct sockaddr *)&client, &client_len);
            if (new_connection == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    break;
                printf(ERRORF "Failed to accept connection.\n");
                return -1;
            }

            int flags = fcntl(new_connection, F_GETFL, 0);
            if (fcntl(new_connection, F_SETFL, flags | O_NONBLOCK) < 0) {
                printf(ERRORF "Failed to set connected socket to non-blocking.\n");
                return -1;
            }
            
            event->data.fd = new_connection;
            event->events = EPOLLIN | EPOLLET;
            epoll_ctl(server->epoll_fd, EPOLL_CTL_ADD, new_connection, event);

        }

        return event->data.fd;

    }

    int handle_request(HTTPServer* server, HTTPRequest request, int client) {

        if (server == NULL) {
            printf(ERRORF "No server object provided.\n");
            return -1;
        }

        bool is_static = false;
        bool is_dynamic = false;
        HTTPResponse result = empty_http_response();

        for (size_t r = 0; r < server->route_count; r++) {
            if (!server->routes[r].dynamic && strcmp(server->routes[r].uri, request.uri) == 0) {
                result = server->routes[r].route_handler(server, &request, NULL, 0);
                is_static = true;
                break;
            }
        }

        for (size_t r = 0; !is_static && r < server->route_count; r++) {
            if (server->routes[r].dynamic) {

                bool is_this_route = true;

                size_t route_part_count = 0;
                char** route_parts = chop_uri(server->routes[r].uri, &route_part_count);

                size_t request_part_count = 0;
                char** request_parts = chop_uri(request.uri, &request_part_count);

                size_t dynamic_data_count = 0;
                char** dynamic_data = (char**)malloc(sizeof(char*) * route_part_count);
                if (dynamic_data == NULL) {
                    printf(ERRORF "Failed to allocate memory for dynamic route data.\n");
                    return -1;
                }

                if (request_part_count != route_part_count)
                    continue;

                for (size_t i = 0; i < route_part_count; i++) {
                    
                    if (strcmp(route_parts[i], "[id]") == 0) {
                        dynamic_data[dynamic_data_count++] = request_parts[i];
                        continue;
                    }

                    if (strcmp(route_parts[i], request_parts[i]) != 0) {
                        is_this_route = false;
                        break;
                    }

                }
                
                if (is_this_route) {
                    result = server->routes[r].route_handler(server, &request, dynamic_data, dynamic_data_count);
                    is_dynamic = true;
                }

                free_chopped_uri(route_parts, route_part_count);
                free_chopped_uri(request_parts, request_part_count);
                free(dynamic_data);

                if (is_this_route)
                    break;

            }
        }

        if (!is_dynamic && !is_static)
            if (server->route_404.route_handler != NULL)
                result = server->route_404.route_handler(server, &request, NULL, 0);
        
        write(client, result.response, result.response_length);
        free(result.response);

        return 0;

    }

    Route new_route(char* uri, RouteHandler route_handler, bool dynamic) {

        return (Route) {
            .uri = uri,
            .route_handler = route_handler,
            .dynamic = dynamic
        };

    }

    char** chop_uri(char* src, size_t* slices) {

        if (src == NULL)
            return NULL;
        
        size_t amount = 1;
        string uri = string_from(src);
        char* uri_ptr = uri.data;

        char** result = (char**)malloc(sizeof(char*) * uri.length);
        if (result == NULL) {
            printf(ERRORF "Failed to allocate memory for chopped route.\n");
            return NULL;
        }

        char* token = strtok(uri.data, "/");
        size_t token_len = strlen(token);

        result[0] = (char*)malloc(token_len + 1);
        if (result[0] == NULL) {
            printf(ERRORF "Failed to allocate memory for chopped route portion.\n");
            return NULL;
        }

        memcpy(result[0], token, token_len + 1);

        for (;;) {
            
            token = strtok(NULL, "/");
            if (token == NULL)
                break;

            token_len = strlen(token);
            result[amount] = (char*)malloc(token_len + 1);
            if (result[amount] == NULL) {
                printf(ERRORF "Failed to allocate memory for chopped route portion.\n");
                return NULL;
            }

            memcpy(result[amount], token, token_len + 1);
            amount++;

        }

        free(uri_ptr);

        if (src[strlen(src) - 1]  == '/') {

            result[amount] = (char*)malloc(1);
            if (result[amount] == NULL) {
                printf(ERRORF "Failed to allocate memory for chopped route portion.\n");
                return NULL;
            }

            result[amount] = "";
            amount++;

        }
        
        if (slices != NULL)
            *slices = amount;

        return result;
        
    }

    void free_chopped_uri(char** src, size_t count) {

        if (src == NULL)
            return;

        for (size_t i = 0; i < count; i++) 
            free(src[i]);
        free(src);

    }

    HTTPResponse create_response(ResponseCode code, const char* mime, byte* data, size_t data_length) {

        if (mime == NULL) {
            printf(ERRORF "No mime type provided.\n");
            return empty_http_response();
        }

        if (data == NULL) {
            printf(ERRORF "No data provided.\n");
            return empty_http_response();
        }

        size_t response_length = 1024 + data_length;
        char* response_buffer = (char*)malloc(response_length);
        if (response_buffer == NULL) {
            printf(ERRORF "Failed to allocate memory for response buffer.\n");
            return empty_http_response();
        }

        const char* status = (code == 200) ? "OK" : "BAD";

        if (sprintf(response_buffer,    
                "HTTP/1.1 %d %s\r\n"
                "Content-Type: %s\r\n"
                "Content-Length: %zu\r\n"
                "\r\n"
                "%s",
                code, status, mime, 
                data_length, data
            ) < 0) {
            printf(ERRORF "Failed to write response to buffer.\n");
            return empty_http_response();
        }

        return (HTTPResponse) {
            .code = code,
            .response = response_buffer,
            .response_length = strlen(response_buffer)
        };

    }

    DOMElement create_element(HTMLTag tag, char* props, char* content, size_t child_count, ...) {

        DOMElement* children = (DOMElement*)malloc(sizeof(DOMElement) * child_count);
        if (children == NULL) {
            printf(ERRORF "Failed to allocate memory for DOM Element.\n");
            return empty_dom_element();
        }

        va_list list;
        va_start(list, child_count);

        for (size_t c = 0; c < child_count; c++)
            children[c] = va_arg(list, DOMElement);
        va_end(list);

        string rendered = string_from_format("<%s %s>\n%s\n", HTMLTagNames[tag], props, content);
        for (size_t c = 0; c < child_count; c++)
            string_append_format(&rendered, "%s\n", children[c].rendered.data);
        string_append_format(&rendered, "</%s>", HTMLTagNames[tag]);

        return (DOMElement) {
            
            .tag            = tag,
            .props          = props,
            .content        = content,

            .rendered       = rendered,

            .children       = children,
            .child_count    = child_count

        };

    }

    void free_dom_element(DOMElement* element) {

        if (element == NULL)
            return;

        string_drop(&element->rendered);
        for (size_t c = 0; c < element->child_count; c++)
            free_dom_element(&element->children[c]);
        free(element->children);
        element->child_count = 0;

    }

    string render_page_from_root(DOMElement* root) {

        if (root == NULL)
            return empty_string();

        root->props = NULL;
        root->content = NULL;

        for (size_t c = 0; c < root->child_count; c++)
            free_dom_element(&root->children[c]);
        
        free(root->children);
        
        root->child_count = 0;
        root->children = NULL;
    
        return root->rendered;
    
    }

#endif // CFRAME_IMPLEMENTATION
#endif // CFRAME_H