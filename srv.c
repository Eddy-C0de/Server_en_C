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
#define BUFFER_SIZE 2048

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];

    // Lecture de la requête HTTP
    int bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0'; 
        printf("Reçu du client :\n%s\n", buffer);

        char method[16], path[256];
        sscanf(buffer, "%15s %255s", method, path);
        printf("Méthode : %s, Chemin : %s\n", method, path);

        const char *html_response =
            "<html>"
            "<head><title>Bienvenue</title></head>"
            "<body><h1>Hello, world!</h1><p>Bienvenue sur mon serveur HTTP.</p></body>"
            "</html>";

        char response[BUFFER_SIZE];

        // Gestion de la méthode GET
        if (strcmp(method, "GET") == 0) {
            snprintf(response, sizeof(response),
                     "HTTP/1.1 200 OK\r\n"
                     "Content-Type: text/html\r\n"
                     "Content-Length: %zu\r\n"
                     "\r\n"
                     "%s",
                     strlen(html_response), html_response);
            send(client_socket, response, strlen(response), 0);
        }
        // Gestion de la méthode HEAD
        else if (strcmp(method, "HEAD") == 0) {
            snprintf(response, sizeof(response),
                     "HTTP/1.1 200 OK\r\n"
                     "Content-Type: text/html\r\n"
                     "Content-Length: %zu\r\n"
                     "\r\n",
                     strlen(html_response));
            send(client_socket, response, strlen(response), 0);
        }
        // Gestion de la méthode POST
        else if (strcmp(method, "POST") == 0) {
            char *body = strstr(buffer, "\r\n\r\n");
            if (body != NULL) {
                body += 4; 
                printf("Données reçues dans le POST : %s\n", body);
            }

            snprintf(response, sizeof(response),
                     "HTTP/1.1 200 OK\r\n"
                     "Content-Type: text/plain\r\n"
                     "Content-Length: 20\r\n"
                     "\r\n"
                     "POST reçu avec succès\n");
            send(client_socket, response, strlen(response), 0);
        }

        else {
            const char *response =
                "HTTP/1.1 405 Method Not Allowed\r\n"
                "Content-Type: text/plain\r\n"
                "Content-Length: 23\r\n"
                "\r\n"
                "Method Not Allowed\n";
            send(client_socket, response, strlen(response), 0);
        }
    }

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

