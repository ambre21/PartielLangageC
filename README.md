# Projet : LD_PRELOAD Malware & C2 Server

Par Ambre LAURENT et Cyprien CHAMAGNE en 3SI1

## Description
Ce projet est une implémentation d'un malware utilisant **LD_PRELOAD**, ainsi qu'un serveur de contrôle (C2). Il inclut plusieurs fonctionnalités :
- **Interception des credentials de connection** pour l'exfiltration des informations de connection de l'utilisateur.
- **Système de port knocking**.
- **C2 multithreadé** permettant la communication avec des clients.
- **Techniques de dissimulation** :
  - Cacher des fichiers.

## Arborescence du projet
```
PartielLangageC/
├── malware/
│   ├── malware.c        # Code principal du malware
│   ├── persistence.c    # Gestion de la persistance
│   ├── makefile         # Compilation du malware
│
├── Serveur/
│   ├── port_knocking.c  # Gestion du port knocking
│   ├── script_serveur.c # Serveur de contrôle (C2)
│
├── src/
│   ├── cacher_fichiers.c # Fonctionnalité pour cacher des fichiers
│   ├── block_fic/
│   │   ├── block_fic.c   # Suppression ou blocage des logs
│   │   ├── set_alis.sh   # Script annexe
```

## Dépendances
Le projet utilise les bibliothèques standard suivantes :
- **Réseau** : `arpa/inet.h`, `netdb.h`, `netinet/ip.h`, `netinet/tcp.h`, `sys/socket.h`
- **Fichiers et système** : `dirent.h`, `fcntl.h`, `sys/stat.h`, `sys/types.h`, `unistd.h`
- **Multithreading** : `pthread.h`
- **Gestion des erreurs et signaux** : `errno.h`, `signal.h`
- **Manipulation mémoire et chaînes de caractères** : `string.h`, `stdarg.h`
- **Bibliothèques dynamiques (LD_PRELOAD, hooks)** : `dlfcn.h`
- **Utilitaires** : `stdio.h`, `stdlib.h`, `time.h`

## Installation et Compilation
### Malware
```sh
cd malware
make
```

### Serveur
```sh
cd Serveur
gcc -o serveur script_serveur.c -lpthread
```

## Utilisation
- **Lancer le malware** :
  ```sh
  ./malware
  ```
- **Lancer le serveur C2** :
  ```sh
  ./serveur
  ```

## Fonctionnalités
### LD_PRELOAD Malware
- Interception des codes de connexion de l'utilisateur pour les envoyés au serveur C2
- Persistance via des techniques de manipulation système.
- Techniques d'évasion pour masquer sa présence.

### Port Knocking
- Mécanisme de communication basé sur des séquences de ports pour identifier les connexions légitimes.

### C2 Server
- Serveur TCP multithreadé permettant le contrôle des clients infectés.
- Interface API pour enregistrer les clients et lier des clés SSH.

## Auteurs et Licence
Projet réalisé dans le cadre d'un examen ESGI, par **Ambre** et **Cyprien**.



