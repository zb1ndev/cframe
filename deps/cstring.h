#if !defined(CSTRING_H)
#define CSTRING_H

    #include <stdio.h>      // printf
    #include <stdlib.h>     // malloc
    #include <string.h>     // strlen, memcpy
    #include <stdarg.h>     // va_start, va_end, va_list

    typedef struct string {

        char* data;
        size_t length;
        size_t capacity;

    } string;

    #define     LOGF                        "[\e[030mLOG\e[0m] "
    #define     WARNF                       "[\e[033mWARNING\e[0m] "
    #define     ERRORF                      "[\e[031mERROR\e[0m] "

    #define     empty_string()              (string){NULL,0,0}

    void        string_drop                 (string* ptr);
    int         string_resize               (string* ptr, size_t size);

    string      string_from_until           (char* src, size_t index);                      /* UNSAFE */ 
    string      string_from                 (char* src);
    string      string_from_section         (char* src, size_t start, size_t size);         /* UNSAFE */ 
    string      string_from_format          (const char* fmt, ...);
    string      string_from_size            (size_t size);

    int         string_append_n             (string* ptr, char* src, size_t size);          /* UNSAFE */
    int         string_append               (string* ptr, char* src);
    int         string_append_destroy       (string* ptr, string src);

    #define     string_append_format(ptr, fmt, ...) \
        string_append_destroy(ptr, string_from_format(fmt, __VA_ARGS__))

#if defined(CSTRING_IMPLEMENTATION)

    void string_drop(string* ptr) {

        free(ptr->data);
        *ptr = empty_string();

    }

    int string_resize(string* ptr, size_t size) {

        if (size < 0) {
            printf(WARNF "Please provide a valid size.\n");
            return -1;
        }

        char* temp = (char*)realloc(ptr->data, size);
        if (temp == NULL) {
            printf(ERRORF "Failed to resize string.\n");
            return -1;
        }

        ptr->data = temp;
        ptr->capacity = size;

        return 0;

    }


    /* UNSAFE */ string string_from_until(char* src, size_t index) {

        if (index < 0) {
            printf(WARNF "Please provide a valid index.\n");
            return empty_string();
        }

        string result = empty_string();

        char* temp = (char*)malloc(index + 1);
        if (temp == NULL) {
            printf(ERRORF "Failed to allocate space for new string.\n");
            return empty_string();
        }

        result.data = temp;
        memcpy(result.data, src, index);
        result.data[index] = '\0';
        
        result.length = index;
        result.capacity = index + 1;

        return result;

    }


    string string_from(char* src) {

        size_t src_length = strlen(src);
        return string_from_until(src, src_length);

    }

    /* UNSAFE */ string string_from_section(char* src, size_t start, size_t size) {

        return string_from_until(src + start, size);
    
    }

    string string_from_format(const char* fmt, ...) {

        va_list args;
        string result = empty_string();

        va_start(args, fmt);
        int size = vsnprintf(result.data, result.length, fmt, args);
        va_end(args);

        if (size < 0) {
            printf(ERRORF "Failed to read required length of string for format.\n");
            return empty_string();
        }

        result.length = (size_t)size;
        result.capacity = result.length + 1;

        result.data = (char*)malloc(result.capacity);
        if (result.data == NULL) {
            printf(ERRORF "Failed to allocate memory for string.\n");
            return empty_string();
        }

        va_start(args, fmt);
        size = vsnprintf(result.data, (result.length + 1), fmt, args);
        va_end(args);

        if (size < 0) {
            printf(ERRORF "Failed to create string from format.\n");
            return empty_string();
        }

        return result;

    }

    string string_from_size(size_t size) {

        if (size < 0) {
            printf(WARNF "Please provide a valid size.\n");
            return empty_string();
        }

        string result = empty_string();
        if (string_resize(&result, size) < 0)
            return empty_string();
        return result;

    }

    /* UNSAFE */ int string_append_n(string* ptr, char* src, size_t size) {

        if (size < 0) {
            printf(WARNF "Please provide a valid size.\n");
            return -1;
        }


        if ((ptr->length + size + 1) > ptr->capacity)
            string_resize(ptr, ptr->length + size + 1);

        char* buffer = malloc(ptr->length + 1);
        if (buffer == NULL) {
            printf(ERRORF "Failed to allocate space for buffer.\n");
            return -1;
        }
        memcpy(buffer, ptr->data, ptr->length + 1);

        int amnt = snprintf(ptr->data, ptr->length + size + 1, "%s%s", buffer, src);
        ptr->length = (ptr->length + size);

        return (amnt < 0) ? -1 : 0;

    } 

    int string_append(string* ptr, char* src) {

        return string_append_n(ptr, src, strlen(src));

    } 

    int string_append_destroy(string* ptr, string src) {

        if (string_append(ptr, src.data) < 0) {
            printf(ERRORF "Failed to append string.\n");
            string_drop(&src);
            return -1;
        }
        
        string_drop(&src);
        return 0;

    }

#endif // CSTRING_IMPLEMENTATION
#endif // CSTRING_H