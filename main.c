#include "signal.h"
#include <arpa/inet.h>
#include <complex.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 4096 * 16
#define TIMEOUT_MS 500

volatile sig_atomic_t running = true;

void handle_signal(int signal) {
  if (signal == SIGINT || signal == SIGTERM) {
    running = false;
  }
}

#define MEOWS_SIZE 10
char *meows[MEOWS_SIZE] = {"MEOW",
                           "mew :3",
                           "mowwwwwwowowowowowowowo",
                           "wiiiiiiiwiwiwiwiwiwiwiwiwiwiwiwiiwiwiwi",
                           "mow",
                           "miau =^_^=",
                           " /\\_/\\\n"
                           "( o.o )\n"
                           " > ^ <",
                           "mrrrow  0_o",
                           "    /\\_____/\\\n"
                           "   /  o   o  \\\n"
                           "  ( ==  ^  == )\n"
                           "   )         (\n"
                           "  (           )\n"
                           " ( (  )   (  ) )\n"
                           "(__(__)___(__)__)\n",
                           "|\\---/|\n"
                           "| o_o |\n"
                           " \\_^_/\n"};
int server_fd;
struct sockaddr_in server_addr;

void *handle_client(void *arg) {
  int client_fd = *((int *)arg);
  free(arg);
  char *buffer = (char *)malloc(BUFFER_SIZE * sizeof(char));

  ssize_t received;

  for (;;) {
    received = recv(client_fd, buffer, BUFFER_SIZE, 0);
    if (received <= 0 || (received == 4 && strncmp(buffer, "quit", 4)))
      break;
    size_t response_idx = rand() % MEOWS_SIZE;
    char *response = meows[response_idx];
    size_t response_len = strlen(response);
    char *w_newline = (char *)malloc(response_len + 2);

    strcpy(w_newline, response);
    strcat(w_newline, "\n");
    send(client_fd, w_newline, response_len + 2, 0);
  }

  free(buffer);
  close(client_fd);

  return NULL;
}

int main() {
  signal(SIGINT, handle_signal);
  signal(SIGTERM, handle_signal);

  // create server  socket
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Socket failed");
    exit(EXIT_FAILURE);
  }

  // Configure server
  struct sockaddr_in server_addr = {.sin_family = AF_INET,
                                    .sin_addr = {.s_addr = INADDR_ANY},
                                    .sin_port = htons(PORT)};

  // Bind config
  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    perror("Bind failed");
    exit(EXIT_FAILURE);
  }

  // Listen for connections
  if (listen(server_fd, 10) < 0) {
    perror("listen failed");
    exit(EXIT_FAILURE);
  }

  printf("Starting server =^_^=\n");
  while (running) {
    // client info
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int *client_fd = malloc(sizeof(int));

    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(server_fd, &read_fds);

    struct timeval timeout = {.tv_sec = 0, .tv_usec = 1000 * TIMEOUT_MS};

    int ready = select(server_fd + 1, &read_fds, NULL, NULL, &timeout);

    if (ready == -1) {
      free(client_fd);
      continue;
    } else if (ready == 0) {
      if (!running)
        break;
      continue;
    }

    if ((*client_fd = accept(server_fd, (struct sockaddr *)&client_addr,
                             &client_addr_len)) == -1) {
      perror("accept failed");
      free(client_fd);
      break;
    }

    printf("Accepted connection from %s::%d\n", inet_ntoa(client_addr.sin_addr),
           ntohs(client_addr.sin_port));

    handle_client((void *)client_fd);
  }

  printf("Closing server :3\n");
  close(server_fd);

  return 0;
}
