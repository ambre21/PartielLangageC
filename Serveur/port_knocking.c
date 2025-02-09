#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

#define MAX_CLIENTS 10
#define KNOCK_SEQUENCE {1234, 5678, 9101}
#define SEQ_LENGTH 3
#define TIMEOUT 10

typedef struct {
    struct in_addr ip;
    int sequence[SEQ_LENGTH];
    int index;
    time_t last_knock;
} KnockSession;

KnockSession clients[MAX_CLIENTS] = {0};

// Vérifie si une IP est déjà enregistrée et met à jour la séquence
int update_knock_sequence(struct in_addr ip, int port) {
    time_t now = time(NULL);
    int knock_seq[SEQ_LENGTH] = KNOCK_SEQUENCE;

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].ip.s_addr == ip.s_addr) {
            if (now - clients[i].last_knock > TIMEOUT) {
                // Reset si le délai est dépassé
                memset(&clients[i], 0, sizeof(KnockSession));
            }
            if (clients[i].sequence[clients[i].index] == knock_seq[clients[i].index]) {
                clients[i].sequence[clients[i].index] = port;
                clients[i].last_knock = now;
                clients[i].index++;

                if (clients[i].index == SEQ_LENGTH) {
                    return 1; // Séquence correcte
                }
            } else {
                memset(&clients[i], 0, sizeof(KnockSession)); // Reset si erreur
            }
            return 0;
        }
    }

    // Ajoute une nouvelle IP si non trouvée
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].ip.s_addr == 0) {
            clients[i].ip = ip;
            clients[i].sequence[0] = port;
            clients[i].index = 1;
            clients[i].last_knock = now;
            return 0;
        }
    }
    return 0;
}

// Ouvre le port SSH avec iptables
void open_port(struct in_addr ip) {
    char command[256];
    sprintf(command, "iptables -A INPUT -s %s -p tcp --dport 22 -j ACCEPT", inet_ntoa(ip));
    system(command);
    printf("Port ouvert pour %s\n", inet_ntoa(ip));
}

int main() {
    int sock;
    struct sockaddr_in source;
    socklen_t addr_len = sizeof(source);
    char buffer[4096];

    // Créer un socket brut pour écouter le trafic TCP
    sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sock < 0) {
        perror("Erreur socket");
        exit(1);
    }

    printf("Écoute du port knocking...\n");

    while (1) {
        ssize_t data_size = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&source, &addr_len);
        if (data_size < 0) continue;

        struct ip *ip_header = (struct ip *)buffer;
        struct tcphdr *tcp_header = (struct tcphdr *)(buffer + (ip_header->ip_hl * 4));

        if (tcp_header->syn) {  // Détection des paquets SYN (début de connexion)
            printf("Connexion détectée : %s:%d\n", inet_ntoa(ip_header->ip_src), ntohs(tcp_header->th_dport));

            if (update_knock_sequence(ip_header->ip_src, ntohs(tcp_header->th_dport))) {
                open_port(ip_header->ip_src);
            }
        }
    }

    close(sock);
    return 0;
}
