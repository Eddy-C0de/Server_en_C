/* srv.c */

#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#define PORT 8000
#define BUFFER_SIZE 512

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    int bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        printf("Reçu du client : %s\n", buffer);
    }

    const char *response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 13\r\n"
        "\r\n"
        "Hello, world!";
    send(client_socket, response, strlen(response), 0);
	close(client_socket);
}

// Gestion des processus zombies
void reap_zombies(int sig) {
    (void)sig; // Évite les avertissements 
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t taille_addr = sizeof(client_addr);

    // Nettoyer les processus fils
    signal(SIGCHLD, reap_zombies);

    // Création du socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("socket");
        return -1;
    }

    // Configuration de l'adresse du serveur
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	// J'assigne une adresse au socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(server_socket);
        return -1;
    }

    if (listen(server_socket, 5) < 0) {
        perror("listen");
        close(server_socket);
        return -1;
    }

    printf("En écoute sur 127.0.0.1:%d\n", PORT);

	// Connexions multiples
    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &taille_addr);
        if (client_socket < 0) {
            perror("accept");
            continue;
        }

        printf("Client connecté depuis %s:%d\n",
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // Création d'un processus fils pour gérer le client
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            close(client_socket);
        } else if (pid == 0) {
            close(server_socket); // Le fils n'a pas besoin du socket serveur
            handle_client(client_socket);
            exit(0); 
        } else {
            close(client_socket);
        }
    }

    close(server_socket);

    return 0;
}

