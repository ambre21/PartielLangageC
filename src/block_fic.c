#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <dlfcn.h>
#include <string.h>
#include <stdarg.h>

// Définir le fichier à bloquer
#define BLOCKED_FILE "/home/victim/Bureau/test.txt"

// Déclaration de la fonction d'origine
int (*original_open)(const char *pathname, int flags, ...) = NULL;

// Implémentation de notre hook pour open()
int open(const char *pathname, int flags, ...) {
    // Afficher un message de debug
    printf("Tentative d'ouverture de : %s\n", pathname);

    // Vérifier si le fichier est celui que l'on veut bloquer
    if (pathname && strcmp(pathname, BLOCKED_FILE) == 0) {
        printf("L'accès à %s est bloqué !\n", pathname);
        return -1;  // Bloque l'accès
    }

    // Appeler la fonction d'origine avec les arguments variadiques
    if (original_open == NULL) {
        original_open = dlsym(RTLD_NEXT, "open");
    }

    va_list args;
    va_start(args, flags);

    int result = original_open(pathname, flags, args);

    va_end(args);
    return result;
}
