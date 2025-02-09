# Fonctionnement d'un Linker, de LD_PRELOAD et des Threads sous Linux

## 1. Le Linker : Rôle et Fonctionnement

Un **linker** (ou éditeur de liens) est un programme qui combine plusieurs fichiers objets (éventuellement compilés à partir de différentes sources) pour produire un exécutable. Il assure la résolution des symboles et la gestion des dépendances.

### 1.1 Types de Linking

- **Linking statique** : Toutes les bibliothèques nécessaires sont incluses directement dans l'exécutable final. Cela augmente la taille du fichier, mais le rend indépendant d'éventuelles modifications des bibliothèques système.
- **Linking dynamique** : L'exécutable ne contient que les références aux bibliothèques, qui seront chargées à l'exécution. Cela permet des mises à jour sans recompiler l'application et réduit la taille des fichiers.

### 1.2 Fonctionnement du Linking Dynamique

Quand un programme utilise des bibliothèques dynamiques, le linker dynamique (`ld.so` ou `ld-linux.so`) est responsable de leur chargement. Il va chercher les fichiers dans des emplacements comme :

- `/lib`
- `/usr/lib`
- D'autres répertoires définis par `LD_LIBRARY_PATH`

Exemple de linking dynamique :

```sh
gcc -o mon_programme mon_programme.c -lmylib
```

On peut utiliser `ldd` pour voir les bibliothèques liées :

```sh
ldd mon_programme
```

## 2. LD_PRELOAD : Un Outil de Surcharge de Bibliothèques

`LD_PRELOAD` est une variable d'environnement permettant de charger une bibliothèque avant celles normalement liées par un programme. Cela permet d'intercepter des appels à des fonctions et de les modifier sans toucher au code source.

### 2.1 Comment fonctionne `LD_PRELOAD` ?

Lorsque `LD_PRELOAD` est défini, le linker dynamique charge la bibliothèque précisée avant toutes les autres. Cela signifie que si une fonction est définie à la fois dans la bibliothèque standard et dans la bibliothèque préchargée, cette dernière sera utilisée.

Cela permet, entre autres :
- D'intercepter et modifier le comportement de fonctions système comme `malloc`, `open`, `read`, etc.
- De tester des correctifs sans recompiler l'application.
- De déboguer ou tracer l'exécution d'un programme.
- De manipuler des binaires fermés en modifiant leur comportement au niveau des appels de fonctions.

### 2.2 Exemple d'utilisation : Interception de `malloc`

Un exemple classique est la surcharge de `malloc` :

```c
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

void* malloc(size_t size) {
    static void* (*real_malloc)(size_t) = NULL;
    if (!real_malloc) {
        real_malloc = dlsym(RTLD_NEXT, "malloc");
    }
    printf("malloc intercepté : %zu octets\n", size);
    return real_malloc(size);
}
```

Compilation :

```sh
gcc -shared -fPIC -o mymalloc.so mymalloc.c -ldl
```

Injection dans un programme :

```sh
LD_PRELOAD=./mymalloc.so ./mon_programme
```

### 2.3 Autres utilisations pratiques de `LD_PRELOAD`

- **Forcer une application à utiliser une version spécifique d'une bibliothèque**
  ```sh
  LD_PRELOAD=/path/to/custom/libmylib.so ./mon_programme
  ```
- **Déboguer un programme en ajoutant des logs sur les appels système**
- **Restreindre certaines actions (ex: bloquer l'accès à un fichier en redéfinissant `open`)**
- **Instrumentation de programmes pour collecter des statistiques sans modifier leur code source**

## 3. Les Threads sous Linux

Un **thread** est une unité d'exécution au sein d'un processus. Contrairement aux processus, les threads partagent la même mémoire et peuvent donc communiquer plus efficacement.

### 3.1 Création de Threads avec `pthread`

Sous Linux, on utilise la bibliothèque POSIX `pthread`.

Exemple simple :

```c
#include <stdio.h>
#include <pthread.h>

void* thread_function(void* arg) {
    printf("Thread en cours d'exécution\n");
    return NULL;
}

int main() {
    pthread_t thread;
    pthread_create(&thread, NULL, thread_function, NULL);
    pthread_join(thread, NULL);
    return 0;
}
```

Compilation :

```sh
gcc -o thread_example thread_example.c -pthread
```

### 3.2 Synchronisation des Threads

Problème classique : plusieurs threads modifient une variable en même temps, causant des erreurs.

Solution : les **mutex**.

```c
#include <stdio.h>
#include <pthread.h>

pthread_mutex_t lock;
int compteur = 0;

void* thread_function(void* arg) {
    pthread_mutex_lock(&lock);
    compteur++;
    printf("Compteur : %d\n", compteur);
    pthread_mutex_unlock(&lock);
    return NULL;
}

int main() {
    pthread_t thread1, thread2;
    pthread_mutex_init(&lock, NULL);
    pthread_create(&thread1, NULL, thread_function, NULL);
    pthread_create(&thread2, NULL, thread_function, NULL);
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_mutex_destroy(&lock);
    return 0;
}
```

## Conclusion

- **Le linker** lie les fichiers objets pour créer un exécutable et charge les bibliothèques dynamiques au besoin.
- **LD_PRELOAD** permet de forcer le chargement d'une bibliothèque spécifique pour redéfinir des fonctions, facilitant l'interception et la modification d'appels système.
- **Les threads** permettent l'exécution concurrente, mais doivent être synchronisés pour éviter les problèmes d'accès concurrent.

## Sources

- "Linkers and Loaders" - John R. Levine
- `man ld`, `man ldd`, `man pthread`
- Documentation GNU C Library: https://www.gnu.org/software/libc/manual/html_mono/libc.html
- Linux Programmer’s Manual: https://linux.die.net/man/
- "What Is the LD_PRELOAD Trick?" - Baeldung: https://www.baeldung.com/linux/ld_preload-trick-what-is
- "A Simple LD_PRELOAD Tutorial" - Catonmat: https://catonmat.net/simple-ld-preload-tutorial
- "Awesome LD_PRELOAD" - GitHub: https://github.com/gaul/awesome-ld-preload

---

