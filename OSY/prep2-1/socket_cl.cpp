// ***************************************************************************
//
// Program example for subject Operating Systems
//
// Petr Olivka, Dept. of Computer Science, petr.olivka@vsb.cz, 2026
//
// Example of socket server/client.
//
// This program is example of socket client.
// The mandatory arguments of program is IP address or name of server and
// a port number.
//
// ***************************************************************************

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <cerrno>
#include <unistd.h>
#include <poll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

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
    "  Socket client example.\n"
    "\n"
    "  Use: %s [-h] [-d] ip_or_name port_number\n"
    "\n"
    "    -d  debug mode \n"
    "    -h  this help\n"
    "\n"
    ;

// ***************************************************************************

int main(int argc, char **argv)
{
    int portNumber = 0;
    char *hostName = nullptr;

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
            if (hostName == nullptr)
            {
                hostName = argv[argumentIndex];
            }
            else if (portNumber == 0)
            {
                portNumber = atoi(argv[argumentIndex]);
            }
        }
    }

    if (hostName == nullptr || portNumber <= 0)
    {
        logMessage(LOG_INFO, "Host or port is missing!");
        printf(helpText, argv[0]);
        exit(EXIT_FAILURE);
    }

    logMessage(LOG_INFO, "Connection to '%s':%d.", hostName, portNumber);

    addrinfo addressHints, *addressResults;
    bzero(&addressHints, sizeof(addressHints));
    addressHints.ai_family = AF_INET;
    addressHints.ai_socktype = SOCK_STREAM;

    int addressLookupResult = getaddrinfo(hostName, nullptr, &addressHints, &addressResults);
    if (addressLookupResult)
    {
        logMessage(LOG_ERROR, "Unknown host name!");
        exit(EXIT_FAILURE);
    }

    // get the first address
    sockaddr_in socketAddress = *(sockaddr_in *)addressResults->ai_addr;
    socketAddress.sin_port = htons(portNumber);
    freeaddrinfo(addressResults);

    // socket creation
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0)
    {
        logMessage(LOG_ERROR, "Unable to create socket.");
        exit(EXIT_FAILURE);
    }

    // connect to server
    if (connect(serverSocket, (sockaddr *)&socketAddress, sizeof(socketAddress)) < 0)
    {
        logMessage(LOG_ERROR, "Unable to connect server.");
        exit(EXIT_FAILURE);
    }

    uint addressLength = sizeof(socketAddress);
    // my IP
    getsockname(serverSocket, (sockaddr *)&socketAddress, &addressLength);
    logMessage(LOG_INFO, "My IP: '%s'  port: %d", inet_ntoa(socketAddress.sin_addr), ntohs(socketAddress.sin_port));
    // server IP
    getpeername(serverSocket, (sockaddr *)&socketAddress, &addressLength);
    logMessage(LOG_INFO, "Server IP: '%s'  port: %d", inet_ntoa(socketAddress.sin_addr), ntohs(socketAddress.sin_port));

    logMessage(LOG_INFO, "Enter 'quit' or 'close' to close application.");

    // list of fd sources
    pollfd pollDescriptors[2];

    pollDescriptors[0].fd = STDIN_FILENO;
    pollDescriptors[0].events = POLLIN;
    pollDescriptors[1].fd = serverSocket;
    pollDescriptors[1].events = POLLIN;

    // Give each client its own sequence of generated examples.
    srand(getpid());

    // go!
    while (1)
    {
        char buffer[128] = {};

        // select from fds
        int pollResult = poll(pollDescriptors, 2, 5000);
        if (pollResult < 0) break;

        // After five seconds without activity, send a simple example.
        if (pollResult == 0)
        {
            int firstNumber = rand() % 10;
            int secondNumber = rand() % 10;
            char operation = rand() % 2 ? '+' : '-';
            int byteCount = snprintf(buffer, sizeof(buffer), "%s%d%c%d\n", STR_REQUEST, firstNumber, operation, secondNumber);

            logMessage(LOG_INFO, "Timeout: sending REQ: %d%c%d.", firstNumber, operation, secondNumber);
            byteCount = write(serverSocket, buffer, byteCount);
            if (byteCount < 0)
            {
                logMessage(LOG_ERROR, "Unable to send generated example to server.");
                break;
            }
            continue;
        }

        // data on stdin?
        if (pollDescriptors[0].revents & POLLIN)
        {
            //  read from stdin
            int byteCount = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);
            if (byteCount == 0)
            {
                logMessage(LOG_DEBUG, "Stdin closed.");
                break;
            }
            if (byteCount < 0)
            {
                logMessage(LOG_ERROR, "Unable to read from stdin.");
                break;
            }
            else
            {
                logMessage(LOG_DEBUG, "Read %d bytes from stdin.", byteCount);
            }

            // send data to server
            // Add the request prefix to each line, including pasted lines.
            bool closeRequested = false;
            char *expression = strtok(buffer, "\r\n");
            while (expression != nullptr)
            {
                char request[256];
                if (!strcasecmp(expression, STR_QUIT) || !strcasecmp(expression, STR_CLOSE))
                {
                    snprintf(request, sizeof(request), "%s\n", expression);
                    closeRequested = true;
                }
                else
                {
                    snprintf(request, sizeof(request), "%s%s\n", STR_REQUEST, expression);
                }

                byteCount = write(serverSocket, request, strlen(request));
                if (byteCount < 0)
                {
                    logMessage(LOG_ERROR, "Unable to send data to server.");
                    closeRequested = true;
                    break;
                }
                logMessage(LOG_DEBUG, "Sent %d bytes to server.", byteCount);

                if (closeRequested)
                {
                    break;
                }
                expression = strtok(nullptr, "\r\n");
            }
            if (closeRequested)
            {
                break;
            }
        }

        // data from server?
        if (pollDescriptors[1].revents & POLLIN)
        {
            // read data from server
            int byteCount = read(serverSocket, buffer, sizeof(buffer));
            if (byteCount == 0)
            {
                logMessage(LOG_DEBUG, "Server closed socket.");
                break;
            }
            else if (byteCount < 0)
            {
                logMessage(LOG_ERROR, "Unable to read data from server.");
                break;
            }
            else
            {
                logMessage(LOG_DEBUG, "Read %d bytes from server.", byteCount);
            }

            // display on stdout
            byteCount = write(STDOUT_FILENO, buffer, byteCount);
            if (byteCount < 0)
            {
                logMessage(LOG_ERROR, "Unable to write to stdout.");
                break;
            }

            // request to close?
            if (strncasecmp(buffer, STR_CLOSE, strlen(STR_CLOSE)) == 0)
            {
                logMessage(LOG_INFO, "Connection will be closed...");
                break;
            }
        }
    }

    // close socket
    close(serverSocket);

    return 0;
} // main

