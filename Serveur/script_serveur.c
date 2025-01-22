#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080
#define MAX_CLIENTS 10

// Structure pour stocker les informations de chaque client
typedef struct {
    int sock;
    struct sockaddr_in address;
} client_t;

// Fonction pour gérer la connexion d'un client
void *handle_client(void *arg) {
    client_t *client = (client_t *)arg;
    char buffer[1024];
    int bytes_received;

    // Afficher un message de connexion réussie
    printf("Client connecté : %s:%d\n", inet_ntoa(client->address.sin_addr), ntohs(client->address.sin_port));

    // Recevoir un message du client
    while ((bytes_received = recv(client->sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_received] = '\0'; // Terminer la chaîne
        printf("Message reçu : %s\n", buffer);
        send(client->sock, "Message reçu\n", 13, 0); // Répondre au client
    }

    if (bytes_received == 0) {
        printf("Client déconnecté : %s:%d\n", inet_ntoa(client->address.sin_addr), ntohs(client->address.sin_port));
    } else if (bytes_received == -1) {
        perror("recv");
    }

    // Fermer la connexion
    close(client->sock);
    free(client);
    return NULL;
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    pthread_t thread_id;

    // Créer le socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Configurer l'adresse du serveur
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Lier le socket à l'adresse et au port
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // Écouter les connexions entrantes
    if (listen(server_fd, MAX_CLIENTS) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Serveur en écoute sur le port %d...\n", PORT);

    // Accepter les connexions des clients
    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd == -1) {
            perror("accept");
            continue;
        }

        // Allouer un objet client
        client_t *client = (client_t *)malloc(sizeof(client_t));
        client->sock = client_fd;
        client->address = client_addr;

        // Créer un thread pour gérer la connexion du client
        if (pthread_create(&thread_id, NULL, handle_client, (void *)client) != 0) {
            perror("pthread_create");
            close(client_fd);
            free(client);
        } else {
            pthread_detach(thread_id); // Détacher le thread pour ne pas avoir à attendre sa fin
        }
    }

    close(server_fd);
    return 0;
}
