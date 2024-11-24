/* srv.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT 8000

int main() {

	int sct, req; 
	struct sockaddr_in srv, client;
	socklen_t taille_addr;
	char buffer[512];
	char *data;

	//  Reservation de la memoire pour srv et client 
	memset(&srv, 0, sizeof(srv));
	memset(&client, 0, sizeof(client));

	// Création du socket :)
	sct = socket(AF_INET, SOCK_STREAM, 0);
	if (sct < 0) {
		printf("erreur de socket()\n");
		return -1;
	}
	srv.sin_addr.s_addr = inet_addr("127.0.0.1");
	srv.sin_port = htons(PORT); 
	srv.sin_family = AF_INET; 

	// J'assigne une local addresse à la socket
	if (bind(sct, (struct sockaddr *)&srv, sizeof(srv))) {
		printf("erreur lors du bind()\n");
		return -1; 	
	}

	// On se met en écoute avant d'accepter les connexions !!
	if (listen(sct, 5)) {
		printf("erreur lors du listen()\n");
		close(sct);
		return -1;
	}
	printf("En écoute sur 127.0.0.1:%d\n", PORT);
	
	// Connexions multiples
	while (1) {
		req = accept(sct, (struct sockaddr *)&srv, &taille_addr);
		if (req < 0) {
			printf("erreur lors de accept()\n"); 
			close(sct);
			return -1; 
		}
	}

	printf("Client connecté\n"); 
	
	// Envoie de données au client 
	read(req, buffer, 512); 
	data = "httpd v1.0\n";
	write(req, data, strlen(data));

	close(req);
	close(sct);

	return 0;
}
