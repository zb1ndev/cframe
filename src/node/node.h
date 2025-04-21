#if !defined(NODE_H)
#define NODE_H

    #include "../common.h"

    #if defined(_WIN32)

        typedef struct NodeInstance {
    
            char available;

            STARTUPINFO start_info;
            PROCESS_INFORMATION process_info;
            SECURITY_ATTRIBUTES attributes;

            HANDLE child_read_stdout, child_write_stdout; 
            HANDLE child_read_stdin, child_write_stdin;

        } NodeInstance;

    #endif // _WIN32

    #if defined(unix)

        typedef struct NodeInstance {
    
            char available;

            pid_t pid;
            int child_stdout_pipe[2]; // [0] = read, [1] = write
            int child_stdin_pipe[2];  // [0] = read, [1] = write

        } NodeInstance;

    #endif // unix

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

#endif // NODE_H