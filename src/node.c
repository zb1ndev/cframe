#include "./include/node.h"

#if defined(_WIN32)

    NodeInstance spawn_node_instance(const char* file) {
    
        NodeInstance result;
        result.attributes = (SECURITY_ATTRIBUTES){ sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };

        if (!CreatePipe(&result.child_read_stdout, &result.child_write_stdout, &result.attributes, 0)) {
            perror("Failed to create stdout pipe.");
            return (NodeInstance){ 0 };
        }
        SetHandleInformation(result.child_read_stdout, HANDLE_FLAG_INHERIT, 0);

        if (!CreatePipe(&result.child_read_stdin, &result.child_write_stdin, &result.attributes, 0)) {
            perror("Failed to create stdout pipe.");
            return (NodeInstance){ 0 };
        }
        SetHandleInformation(result.child_read_stdin, HANDLE_FLAG_INHERIT, 0);

        ZeroMemory(&result.process_info, sizeof(PROCESS_INFORMATION));
        ZeroMemory(&result.start_info, sizeof(STARTUPINFO));

        result.start_info.cb = sizeof(STARTUPINFO);
        result.start_info.hStdError = result.child_write_stdout;
        result.start_info.hStdOutput = result.child_write_stdout;
        result.start_info.hStdInput = result.child_read_stdin;
        result.start_info.dwFlags |= STARTF_USESTDHANDLES;

        String cmd = string_from_format("%s %s", "node.exe", file);
        if (!CreateProcess(NULL, cmd.content, NULL, NULL, TRUE, 0, NULL, NULL, &result.start_info, &result.process_info)) {
            perror("Failed to create node instance.");
            return (NodeInstance){ 0 };
        }

        CloseHandle(result.child_write_stdout);
        CloseHandle(result.child_read_stdin);

        return result;

    }

    void destroy_node_instance(NodeInstance* instance) {

        if (instance->child_read_stdout == NULL) CloseHandle(instance->child_read_stdout);
        if (instance->child_write_stdin == NULL) CloseHandle(instance->child_write_stdin);

        WaitForSingleObject(instance->process_info.hProcess, INFINITE);

        CloseHandle(instance->process_info.hProcess);
        CloseHandle(instance->process_info.hThread);

    }

    String read_node_instance(NodeInstance* instance) {

        String result = string_from("");
        char buffer[128];
        DWORD bytes_read;
        while(ReadFile(instance->child_read_stdout, buffer, sizeof(buffer) - 1, &bytes_read, NULL) && bytes_read > 0) {
            string_append(&result, buffer);
        }
        return result;

    }

    int write_node_instance(NodeInstance* instance, String input) {

        DWORD bytes_written;
        WriteFile(instance->child_write_stdin, input.content, (DWORD)input.length, &bytes_written, NULL);
        CloseHandle(instance->child_write_stdin);
        instance->child_write_stdin = NULL;

        return 0;

    }

#endif // _WIN32

#if defined(unix) 

    NodeInstance spawn_node_instance(const char* file) { 
    
        NodeInstance result = {0};
        
        if (pipe(result.child_stdout_pipe) == -1) {
            perror("Failed to create stdout pipe...");
            return (NodeInstance){0};
        }

        if (pipe(result.child_stdin_pipe) == -1) {
            perror("Failed to create stdin pipe...");
            return (NodeInstance){0};
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("Failed to create node instance...");
            return (NodeInstance){0};
        }

        if (pid == 0) {

            dup2(result.child_stdout_pipe[1], STDOUT_FILENO);
            dup2(result.child_stdin_pipe[0], STDIN_FILENO);
            dup2(result.child_stdout_pipe[1], STDERR_FILENO); 

            execlp("node", "node", file, (char*)NULL);
            perror("Failed to execute node instance...");
            exit(1);

        }

        result.available = 1;
        result.pid = pid;

        close(result.child_stdout_pipe[1]);
        close(result.child_stdin_pipe[0]);
        return result;

    }

    void destroy_node_instance(NodeInstance* instance) {

        if (!instance || !instance->available) return;
        if (instance->child_stdout_pipe[0] >= 0)
            close(instance->child_stdout_pipe[0]);
        if (instance->child_stdin_pipe[1] >= 0)
            close(instance->child_stdin_pipe[1]);

        int status = 0;
        waitpid(instance->pid, &status, 0);
        instance->available = 0;

    }

    String read_node_instance(NodeInstance* instance) {

        String result = string_from("");
        char buffer[128];
        ssize_t bytes_read;

        while ((bytes_read = read(instance->child_stdout_pipe[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytes_read] = '\0';
            string_append(&result, buffer);
        }

        return result;

    }

    int write_node_instance(NodeInstance* instance, String input) {
        
        ssize_t bytes_written = write(instance->child_stdin_pipe[1], input.content, input.length);

        close(instance->child_stdin_pipe[1]);
        instance->child_stdin_pipe[1] = -1;

        return (bytes_written == input.length) ? 0 : -1;

    }

#endif // unix