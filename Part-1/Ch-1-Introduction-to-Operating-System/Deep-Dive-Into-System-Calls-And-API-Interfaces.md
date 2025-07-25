# Understanding Operating Systems: From Basic Concepts to System Calls Implementation

## Introduction

An operating system (OS) serves as the fundamental bridge between computer hardware and user applications. It's the system software that manages hardware resources and provides various services for computer programs. Think of it as a orchestra conductor, coordinating various components to work harmoniously while ensuring efficient resource utilization and system security.

## System Calls and API Interfaces

### Understanding System Calls

System calls are the fundamental interface between an application and the operating system kernel. They provide a way for programs to request services from the operating system.

![history](/images/systemArchOfKernelAndUserSpace.png)

### Categories of System Calls:

1. Process Control
   - fork()
   - exec()
   - exit()
   - wait()

2. File Management
   - open()
   - read()
   - write()
   - close()

3. Device Management
   - ioctl()
   - read()
   - write()

4. Information Maintenance
   - getpid()
   - alarm()
   - sleep()

5. Communication
   - pipe()
   - socket()
   - shmget()
  
# Comprehensive Guide to System Calls: Implementation and Analysis

## Table of Contents
1. [Categories of System Calls](#categories)
2. [Combined Implementation](#combined)
3. [Assembly Analysis](#assembly)
4. [Advanced System Calls](#advanced)

<a name="categories"></a>
## 1. Categories of System Calls

### 1.1 Process Control System Calls

### fork()
Creates a new process by duplicating the calling process.

```c
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("Fork failed");
        return 1;
    } else if (pid == 0) {
        printf("Child process (PID: %d)\n", getpid());
    } else {
        printf("Parent process (PID: %d), child PID: %d\n", getpid(), pid);
        wait(NULL);
    }
    return 0;
}
```

### exec()
Replaces the current process image with a new one.

```c
#include <stdio.h>
#include <unistd.h>

int main() {
    printf("Before exec\n");
    
    char *args[] = {"/bin/ls", "-l", NULL};
    execvp(args[0], args);
    
    // This line only executes if exec fails
    perror("exec failed");
    return 1;
}
```

### exit()
Terminates the calling process.

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    printf("Process starting...\n");
    sleep(1);
    printf("Process exiting...\n");
    exit(0);
    // Code after exit() is never executed
    printf("This won't print\n");
    return 1;
}
```

### 1.2 File Management System Calls

### open(), read(), write(), close()

```c
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main() {
    int fd;
    char buffer[100];
    
    // Open file for writing
    fd = open("test.txt", O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open for writing");
        return 1;
    }
    
    // Write to file
    const char *text = "Hello, File System!\n";
    ssize_t bytes_written = write(fd, text, strlen(text));
    if (bytes_written < 0) {
        perror("write");
        close(fd);
        return 1;
    }
    close(fd);
    
    // Open file for reading
    fd = open("test.txt", O_RDONLY);
    if (fd < 0) {
        perror("open for reading");
        return 1;
    }
    
    // Read from file
    ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    if (bytes_read < 0) {
        perror("read");
        close(fd);
        return 1;
    }
    buffer[bytes_read] = '\0';
    
    printf("Read from file: %s", buffer);
    close(fd);
    
    return 0;
}
```

### 1.3 Device Management System Calls

### ioctl()
Controls device parameters.

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>

int main() {
    struct winsize ws;
    
    // Get terminal size
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) < 0) {
        perror("ioctl");
        return 1;
    }
    
    printf("Terminal size:\n");
    printf("Rows: %d\n", ws.ws_row);
    printf("Columns: %d\n", ws.ws_col);
    
    return 0;
}
```

### 1.4 Information Maintenance System Calls

### getpid(), alarm(), sleep()

```c
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

void alarm_handler(int signum) {
    printf("Alarm received!\n");
}

int main() {
    // Get process ID
    printf("Current process ID: %d\n", getpid());
    
    // Set up alarm handler
    signal(SIGALRM, alarm_handler);
    
    printf("Setting alarm for 2 seconds...\n");
    alarm(2);
    
    printf("Sleeping for 3 seconds...\n");
    sleep(3);
    
    printf("Sleep completed\n");
    return 0;
}
```

### 1.5 Communication System Calls

### pipe(), socket(), shmget()

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/shm.h>
#include <sys/ipc.h>

int main() {
    // Pipe example
    int pipefd[2];
    char pipe_buffer[100];
    
    if (pipe(pipefd) < 0) {
        perror("pipe");
        return 1;
    }
    
    write(pipefd[1], "Hello through pipe!", 18);
    read(pipefd[0], pipe_buffer, 18);
    printf("Pipe message: %s\n", pipe_buffer);
    
    // Socket example (Unix domain socket)
    int sockfd[2];
    char socket_buffer[100];
    
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockfd) < 0) {
        perror("socketpair");
        return 1;
    }
    
    write(sockfd[1], "Hello through socket!", 20);
    read(sockfd[0], socket_buffer, 20);
    printf("Socket message: %s\n", socket_buffer);
    
    // Shared memory example
    key_t key = ftok(".", 'a');
    int shmid = shmget(key, 1024, IPC_CREAT | 0644);
    
    if (shmid < 0) {
        perror("shmget");
        return 1;
    }
    
    char *shm_ptr = shmat(shmid, NULL, 0);
    strcpy(shm_ptr, "Hello through shared memory!");
    printf("Shared memory message: %s\n", shm_ptr);
    
    shmdt(shm_ptr);
    shmctl(shmid, IPC_RMID, NULL);
    
    return 0;
}
```

## 2. Combined Implementation

Here's a comprehensive example that demonstrates all categories of system calls:

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <signal.h>

void alarm_handler(int signum) {
    printf("Alarm triggered!\n");
}

int main() {
    printf("\n=== Process Control ===\n");
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork");
        return 1;
    } else if (pid == 0) {
        // Child process
        printf("Child process (PID: %d)\n", getpid());
        exit(0);
    } else {
        // Parent process
        printf("Parent process (PID: %d)\n", getpid());
        wait(NULL);
    }
    
    printf("\n=== File Management ===\n");
    int fd = open("test.txt", O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open");
        return 1;
    }
    
    const char *text = "Testing file operations\n";
    write(fd, text, strlen(text));
    
    // Reset file pointer to beginning
    lseek(fd, 0, SEEK_SET);
    
    char buffer[100];
    read(fd, buffer, strlen(text));
    buffer[strlen(text)] = '\0';
    printf("Read from file: %s", buffer);
    close(fd);
    
    printf("\n=== Device Management ===\n");
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) >= 0) {
        printf("Terminal size: %dx%d\n", ws.ws_col, ws.ws_row);
    }
    
    printf("\n=== Information Maintenance ===\n");
    printf("Current PID: %d\n", getpid());
    signal(SIGALRM, alarm_handler);
    alarm(1);
    sleep(2);
    
    printf("\n=== Communication ===\n");
    int pipefd[2];
    if (pipe(pipefd) == 0) {
        char pipe_msg[] = "Pipe communication test";
        write(pipefd[1], pipe_msg, strlen(pipe_msg));
        
        char pipe_buffer[100];
        read(pipefd[0], pipe_buffer, strlen(pipe_msg));
        pipe_buffer[strlen(pipe_msg)] = '\0';
        printf("Pipe message: %s\n", pipe_buffer);
        
        close(pipefd[0]);
        close(pipefd[1]);
    }
    
    // Shared memory
    key_t key = ftok(".", 'b');
    int shmid = shmget(key, 1024, IPC_CREAT | 0644);
    if (shmid >= 0) {
        char *shm_ptr = shmat(shmid, NULL, 0);
        strcpy(shm_ptr, "Shared memory test");
        printf("Shared memory: %s\n", shm_ptr);
        shmdt(shm_ptr);
        shmctl(shmid, IPC_RMID, NULL);
    }
    
    return 0;
}
```

<a name="assembly"></a>
## 3. Assembly Analysis

Let's analyze key parts of the combined implementation in assembly (x86_64):

```nasm
; fork() system call
mov    rax, 57       ; syscall number for fork
syscall
cmp    rax, 0        ; compare return value with 0
je     child_code    ; jump if child process
jl     error         ; jump if error

; write() system call
mov    rax, 1        ; syscall number for write
mov    rdi, 1        ; file descriptor (stdout)
mov    rsi, msg      ; pointer to message
mov    rdx, len      ; message length
syscall

; read() system call
mov    rax, 0        ; syscall number for read
mov    rdi, fd       ; file descriptor
mov    rsi, buffer   ; buffer address
mov    rdx, count    ; number of bytes to read
syscall

; open() system call
mov    rax, 2        ; syscall number for open
mov    rdi, filename ; pointer to filename
mov    rsi, flags    ; open flags
mov    rdx, mode     ; file mode
syscall

; pipe() system call
mov    rax, 22       ; syscall number for pipe
mov    rdi, pipefd   ; array for file descriptors
syscall
```

Key points about the assembly:
1. System call numbers are loaded into RAX
2. Arguments are passed in registers (RDI, RSI, RDX)
3. The `syscall` instruction triggers the kernel mode switch
4. Return values are stored in RAX

## 4. Advanced System Calls

### 4.1 select() and poll()

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/poll.h>
#include <fcntl.h>

void demonstrate_select() {
    fd_set readfds;
    struct timeval tv;
    int stdin_fd = STDIN_FILENO;
    
    // Set timeout
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    
    // Clear and set file descriptor set
    FD_ZERO(&readfds);
    FD_SET(stdin_fd, &readfds);
    
    printf("Waiting for input (select)...\n");
    int ret = select(stdin_fd + 1, &readfds, NULL, NULL, &tv);
    
    if (ret < 0) {
        perror("select");
    } else if (ret == 0) {
        printf("Timeout occurred!\n");
    } else {
        if (FD_ISSET(stdin_fd, &readfds)) {
            char buffer[1024];
            read(stdin_fd, buffer, sizeof(buffer));
            printf("Data received: %s", buffer);
        }
    }
}

void demonstrate_poll() {
    struct pollfd fds[1];
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;
    
    printf("Waiting for input (poll)...\n");
    int ret = poll(fds, 1, 5000); // 5 second timeout
    
    if (ret < 0) {
        perror("poll");
    } else if (ret == 0) {
        printf("Timeout occurred!\n");
    } else {
        if (fds[0].revents & POLLIN) {
            char buffer[1024];
            read(STDIN_FILENO, buffer, sizeof(buffer));
            printf("Data received: %s", buffer);
        }
    }
}

int main() {
    printf("Testing select():\n");
    demonstrate_select();
    
    printf("\nTesting poll():\n");
    demonstrate_poll();
    
    return 0;
}
```

### 4.2 Memory Management System Calls

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>

int main() {
    // Anonymous mapping
    size_t size = 4096; // Page size
    void *anon_map = mmap(NULL, size, 
                         PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANONYMOUS,
                         -1, 0);
    
    if (anon_map == MAP_FAILED) {
        perror("mmap (anonymous)");
        return 1;
    }
    
    // Write to the mapped memory
    strcpy(anon_map, "Test anonymous mapping");
    printf("Anonymous mapping: %s\n", (char*)anon_map);
    
    // File-backed mapping
    int fd = open("mmap.txt", O_

### API Interfaces

Modern operating systems provide several layers of API interfaces:

1. System Call Interface
2. Standard C Library
3. Higher-level APIs

## Practical Implementations

Let's explore some practical implementations of system calls and APIs in C.

### 1. Process Creation Example

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

int main() {
    pid_t pid;
    int status;
    
    // Create a new process
    pid = fork();
    
    if (pid < 0) {
        // Error handling
        fprintf(stderr, "Fork failed\n");
        exit(1);
    } else if (pid == 0) {
        // Child process
        printf("Child process (PID: %d)\n", getpid());
        
        // Execute a new program
        char *args[] = {"/bin/ls", "-l", NULL};
        execvp(args[0], args);
        
        // If execvp returns, an error occurred
        perror("execvp failed");
        exit(1);
    } else {
        // Parent process
        printf("Parent process (PID: %d)\n", getpid());
        
        // Wait for child to complete
        wait(&status);
        printf("Child process completed\n");
    }
    
    return 0;
}
```

To compile and run:
```bash
gcc process_creation.c -o process_creation
./process_creation
```

Expected output:
```
Parent process (PID: 1234)
Child process (PID: 1235)
[Directory listing appears here]
Child process completed
```

### 2. File Operations Example

```c
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define BUFFER_SIZE 1024

int main() {
    int fd;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read, bytes_written;
    
    // Create and open a file
    fd = open("test.txt", O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd == -1) {
        perror("Error opening file");
        exit(1);
    }
    
    // Write to file
    const char *message = "Testing file operations with system calls\n";
    bytes_written = write(fd, message, strlen(message));
    if (bytes_written == -1) {
        perror("Error writing to file");
        close(fd);
        exit(1);
    }
    
    // Close the file
    close(fd);
    
    // Open file for reading
    fd = open("test.txt", O_RDONLY);
    if (fd == -1) {
        perror("Error opening file for reading");
        exit(1);
    }
    
    // Read from file
    bytes_read = read(fd, buffer, BUFFER_SIZE - 1);
    if (bytes_read == -1) {
        perror("Error reading from file");
        close(fd);
        exit(1);
    }
    
    // Null terminate the buffer
    buffer[bytes_read] = '\0';
    
    printf("Read from file: %s", buffer);
    
    // Close the file
    close(fd);
    
    return 0;
}
```

To compile and run:
```bash
gcc file_operations.c -o file_operations
./file_operations
```

### 3. Inter-Process Communication Example

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BUFFER_SIZE 256

int main() {
    int pipefd[2];
    pid_t pid;
    char buffer[BUFFER_SIZE];
    
    // Create pipe
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(1);
    }
    
    // Create child process
    pid = fork();
    
    if (pid < 0) {
        perror("fork");
        exit(1);
    }
    
    if (pid == 0) {  // Child process
        close(pipefd[1]);  // Close unused write end
        
        // Read from pipe
        ssize_t bytes_read = read(pipefd[0], buffer, BUFFER_SIZE);
        if (bytes_read > 0) {
            printf("Child received: %s", buffer);
        }
        
        close(pipefd[0]);
        exit(0);
    } else {  // Parent process
        close(pipefd[0]);  // Close unused read end
        
        // Write to pipe
        const char *message = "Hello from parent process!\n";
        write(pipefd[1], message, strlen(message));
        
        close(pipefd[1]);
        wait(NULL);  // Wait for child to finish
    }
    
    return 0;
}
```

To compile and run:
```bash
gcc ipc_example.c -o ipc_example
./ipc_example
```
