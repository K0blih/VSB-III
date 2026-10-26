// ***************************************************************************
//
// Program example for labs in subject Operating Systems
//
// Petr Olivka, Dept. of Computer Science, petr.olivka@vsb.cz, 2026
//
// Example of socket server.
//
// This program serves multiple clients using one child process per connection.
// The mandatory argument of program is port number for listening.
//
// ***************************************************************************

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <cerrno>
#include <signal.h>
#include <unistd.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>

#define STR_CLOSE   "close"
#define STR_QUIT    "quit"
#define STR_REQUEST "REQ: "

// ***************************************************************************
// log messages

#define LOG_ERROR               0       // errors
#define LOG_INFO                1       // information and notifications
#define LOG_DEBUG               2       // debug messages

// debug flag
int debugLevel = LOG_INFO;

void logMessage(int logLevel, const char *format, ...)
{
    const char *outputFormats[] = {
        "ERR: (%d-%s) %s\n",
        "INF: %s\n",
        "DEB: %s\n"
    };

    if (logLevel && logLevel > debugLevel)
    {
        return;
    }

    char buffer[4096];
    va_list arguments;
    va_start(arguments, format);

    vsnprintf(buffer, sizeof(buffer), format, arguments);

    va_end(arguments);

    switch (logLevel)
    {
        case LOG_INFO:
        case LOG_DEBUG:
            fprintf(stdout, outputFormats[logLevel], buffer);
            break;

        case LOG_ERROR:
            fprintf(stderr, outputFormats[logLevel], errno, strerror(errno), buffer);
            break;
    }
}

// ***************************************************************************
// help

const char helpText[] =
    "\n"
    "  Socket server example.\n"
    "\n"
    "  Use: %s [-h] [-d] port_number\n"
    "\n"
    "    -d  debug mode \n"
    "    -h  this help\n"
    "\n"
    ;

// ***************************************************************************

int handleClient(int clientSocket)
{
    const int bufferSize = 256;
    char commands[3][600];
    int exampleCount = 0;
    int processedCount = 0;

    while (1)
    { // communication
        char buffer[bufferSize] = {};

        // read data from socket
        // Read one line, even if TCP joins or splits the client's messages.
        int byteCount = 0;
        while (byteCount < bufferSize - 1)
        {
            int readResult = read(clientSocket, buffer + byteCount, 1);
            if (readResult == 0)
            {
                logMessage(LOG_INFO, "Client disconnected (process %d, processed %d examples).", getpid(), processedCount);
                return EXIT_SUCCESS;
            }
            if (readResult < 0)
            {
                logMessage(LOG_ERROR, "Unable to read data from client.");
                return EXIT_FAILURE;
            }

            byteCount += readResult;
            if (buffer[byteCount - 1] == '\n')
            {
                break;
            }
        }

        logMessage(LOG_DEBUG, "Process %d read %d bytes from client.", getpid(), byteCount);

        // write data to stdout
        byteCount = write(STDOUT_FILENO, buffer, byteCount);
        if (byteCount < 0)
        {
            logMessage(LOG_ERROR, "Unable to write data to stdout.");
            return EXIT_FAILURE;
        }

        // Remove the newline sent from the client's terminal.
        buffer[strcspn(buffer, "\r\n")] = '\0';
        if (buffer[0] == '\0')
        {
            continue;
        }

        // close request?
        if (!strcasecmp(buffer, STR_CLOSE) || !strcasecmp(buffer, STR_QUIT))
        {
            logMessage(LOG_INFO, "Client requested '%s' (process %d, processed %d examples).", buffer, getpid(), processedCount);
            return EXIT_SUCCESS;
        }

        // Arithmetic requests use the format: REQ: 4+5*8
        if (strncmp(buffer, STR_REQUEST, strlen(STR_REQUEST)))
        {
            const char errorMessage[] = "ERR: expected REQ: expression\n";
            if (write(clientSocket, errorMessage, strlen(errorMessage)) < 0)
            {
                logMessage(LOG_ERROR, "Unable to send protocol error to client.");
                return EXIT_FAILURE;
            }
            continue;
        }
        char *expression = buffer + strlen(STR_REQUEST);
        if (expression[0] == '\0')
        {
            continue;
        }

        // Create a Python command, for example: print("4+5*8=", 4+5*8, sep="")
        snprintf(commands[exampleCount], sizeof(commands[exampleCount]), "print(\"%s=\", %s, sep=\"\")\n", expression, expression);
        exampleCount++;

        // Start Python only when three examples have been collected.
        if (exampleCount < 3)
        {
            continue;
        }
        
        int pipeDescriptors[2];
        if (pipe(pipeDescriptors) < 0)
        {
            logMessage(LOG_ERROR, "Unable to create pipe.");
            return EXIT_FAILURE;
        }

        int pythonPid = fork(); 
        if (pythonPid < 0)
        {
            logMessage(LOG_ERROR, "Unable to create Python process.");
            close(pipeDescriptors[0]);
            close(pipeDescriptors[1]);
            return EXIT_FAILURE;
        }
        if (pythonPid == 0)
        {
            // Python reads from the pipe and writes directly to the client.
            close(pipeDescriptors[1]);
            if (dup2(pipeDescriptors[0], STDIN_FILENO) < 0 || dup2(clientSocket, STDOUT_FILENO) < 0)
            {
                logMessage(LOG_ERROR, "Unable to redirect Python input/output.");
                exit(EXIT_FAILURE);
            }
            close(pipeDescriptors[0]);
            close(clientSocket);

            execlp("python3", "python3", nullptr);
            logMessage(LOG_ERROR, "Unable to start python3.");
            exit(EXIT_FAILURE);
        }

        // Send all three commands and close the pipe so Python receives EOF.
        close(pipeDescriptors[0]);
        for (int exampleIndex = 0; exampleIndex < exampleCount; exampleIndex++)
        {
            int commandLength = strlen(commands[exampleIndex]);
            byteCount = write(pipeDescriptors[1], commands[exampleIndex], commandLength);
            if (byteCount < 0)
                break;
        }
        close(pipeDescriptors[1]);
        int pythonStatus = 0;
        int waitResult = waitpid(pythonPid, &pythonStatus, 0);
        if (byteCount < 0)
        {
            logMessage(LOG_ERROR, "Unable to write command to Python.");
            return EXIT_FAILURE;
        }

        // Count completed batches separately for each client.
        if (waitResult > 0 && WIFEXITED(pythonStatus) && WEXITSTATUS(pythonStatus) == 0)
        {
            processedCount += exampleCount;
        }
        logMessage(LOG_INFO, "Client process %d: processed %d examples.", getpid(), processedCount);
        exampleCount = 0;
    } // while communication
}

// ***************************************************************************

int main(int argc, char **argv)
{
    int portNumber = 0;

    // parsing arguments
    for (int argumentIndex = 1; argumentIndex < argc; argumentIndex++)
    {
        if (strcmp(argv[argumentIndex], "-d") == 0)
        {
            debugLevel = LOG_DEBUG;
        }

        if (strcmp(argv[argumentIndex], "-h") == 0)
        {
            printf(helpText, argv[0]);
            exit(EXIT_SUCCESS);
        }

        if (*argv[argumentIndex] != '-')
        {
            if (portNumber == 0)
            {
                portNumber = atoi(argv[argumentIndex]);
            }
        }
    }

    if (portNumber <= 0)
    {
        logMessage(LOG_INFO, "Bad or missing port number %d!", portNumber);
        printf(helpText, argv[0]);
        exit(EXIT_FAILURE);
    }

    logMessage(LOG_INFO, "Server will listen on port: %d.", portNumber);

    // Automatically reap finished children so they do not become zombies.
    signal(SIGCHLD, SIG_IGN);

    // socket creation
    int listeningSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listeningSocket < 0)
    {
        logMessage(LOG_ERROR, "Unable to create socket.");
        exit(EXIT_FAILURE);
    }

    in_addr anyAddress = { INADDR_ANY };
    sockaddr_in socketAddress;
    socketAddress.sin_family = AF_INET;
    socketAddress.sin_port = htons(portNumber);
    socketAddress.sin_addr = anyAddress;

    // Enable the port number reusing
    int reuseAddress = 1;
    if (setsockopt(listeningSocket, SOL_SOCKET, SO_REUSEADDR, &reuseAddress, sizeof(reuseAddress)) < 0)
    {
        logMessage(LOG_ERROR, "Unable to set socket option!");
    }

    // assign port number to socket
    if (bind(listeningSocket, (const sockaddr *)&socketAddress, sizeof(socketAddress)) < 0)
    {
        logMessage(LOG_ERROR, "Bind failed!");
        close(listeningSocket);
        exit(EXIT_FAILURE);
    }

    // listening on set port
    if (listen(listeningSocket, SOMAXCONN) < 0)
    {
        logMessage(LOG_ERROR, "Unable to listen on given port!");
        close(listeningSocket);
        exit(EXIT_FAILURE);
    }

    logMessage(LOG_INFO, "Enter 'quit' to quit server.");

    // Only the parent watches stdin and accepts new clients.

    // list of fd sources
    pollfd pollDescriptors[2] = {};
    pollDescriptors[0].fd = STDIN_FILENO;
    pollDescriptors[0].events = POLLIN;
    pollDescriptors[1].fd = listeningSocket;
    pollDescriptors[1].events = POLLIN;

    // go!
    while (1) // wait for new client
    {
        // select from fds
        if (poll(pollDescriptors, 2, -1) < 0)
        {
            logMessage(LOG_ERROR, "Function poll failed!");
            close(listeningSocket);
            return EXIT_FAILURE;
        }

        if (pollDescriptors[0].revents & (POLLIN | POLLHUP))
        { // data on stdin
            char buffer[128] = {};

            // read data from stdin
            int byteCount = read(STDIN_FILENO, buffer, sizeof(buffer));
            if (byteCount < 0)
            {
                logMessage(LOG_ERROR, "Unable to read from stdin!");
                close(listeningSocket);
                return EXIT_FAILURE;
            }
            // stdin closed or request to quit?
            if (byteCount == 0 || !strncmp(buffer, STR_QUIT, strlen(STR_QUIT)))
            {
                logMessage(LOG_INFO, "Server stopped accepting new clients.");
                close(listeningSocket);
                return EXIT_SUCCESS;
            }
        }

        if (pollDescriptors[1].revents & POLLIN)
        { // new client?
            sockaddr_in clientAddress;
            socklen_t clientAddressLength = sizeof(clientAddress);

            // new connection
            int clientSocket = accept(listeningSocket, (sockaddr *)&clientAddress, &clientAddressLength);
            if (clientSocket < 0)
            {
                logMessage(LOG_ERROR, "Unable to accept new client.");
                continue;
            }

            // client IP
            logMessage(LOG_INFO, "Client IP: '%s'  port: %d", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));

            int childPid = fork();
            if (childPid < 0)
            {
                logMessage(LOG_ERROR, "Unable to create child process.");
                close(clientSocket);
                continue;
            }
            if (childPid == 0)
            {
                // The child owns this connection and does not accept new ones.
                close(listeningSocket);
                // Wait for Python processes explicitly inside handleClient.
                signal(SIGCHLD, SIG_DFL);
                int result = handleClient(clientSocket);
                close(clientSocket);
                exit(result);
            }

            // The parent must release its copy of the client's socket.
            close(clientSocket);
            logMessage(LOG_INFO, "Client handled by process %d. Waiting for new clients.", childPid);
        }
    } // while wait for client
} // main

