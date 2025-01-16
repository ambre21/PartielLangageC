# PartielLangageC

### **Semaine 1 : Préparation et LD_PRELOAD**

#### **Jour 1 : Comprendre LD_PRELOAD et préparer l’environnement**
- Installe les outils nécessaires (compilateur GCC, éditeur de texte, environnement de test).
- Lis rapidement un ou deux tutoriels sur LD_PRELOAD pour comprendre le fonctionnement de base (exemple : [man ld.so](https://man7.org/linux/man-pages/man8/ld.so.8.html)).
- Lance un exemple simple avec LD_PRELOAD, comme rediriger une fonction standard (`printf`, `malloc`).

#### **Jour 2 : Planification des hooks**
- Identifie les fonctions système nécessaires pour ton projet (par exemple, `open`, `read`, `opendir`, `readdir` pour cacher un fichier).
- Note exactement ce que tu veux modifier : comportement, logs ou affichage.

#### **Jour 3-4 : Implémentation basique de LD_PRELOAD**
- Implémente un hook pour rediriger une fonction simple (par exemple, bloquer l'accès à un fichier spécifique avec `open`).
- Teste avec des fichiers locaux pour vérifier que le comportement modifié fonctionne.
- Ajoute des messages de debug pour observer ce qui se passe.

---

### **Semaine 2 : Command & Control (C2)**

#### **Jour 5-6 : Mise en place du serveur basique**
- Implémente un serveur TCP multithread qui peut :
  - Accepter des connexions de plusieurs "clients".
  - Afficher un message de connexion réussi.
- Teste avec des clients simples (`telnet` ou `nc`) pour t'assurer que le serveur répond correctement.

#### **Jour 7 : Ajout des fonctionnalités de base au C2**
- Ajoute une fonctionnalité pour enregistrer les "clients" connectés.
- Stocke temporairement des informations (par exemple, les adresses IP des clients ou des identifiants fictifs).

#### **Jour 8-9 : Intégration avec LD_PRELOAD**
- Connecte ton LD_PRELOAD avec le serveur C2 :
  - Par exemple, en envoyant au serveur un message quand un fichier est ouvert ou caché.
  - Vérifie que le serveur peut collecter ces données.

---

### **Semaine 3 : Furtivité et finalisation**

#### **Jour 10-11 : Implémentation de la furtivité (Cacher un fichier)**
- Modifie les fonctions `readdir` ou `opendir` pour exclure un fichier spécifique des listes.
- Teste dans des scénarios réels pour t’assurer que le fichier reste invisible (y compris avec des commandes comme `ls`).

#### **Jour 12 : Tests approfondis**
- Combine LD_PRELOAD et le serveur C2 :
  - Teste si le programme modifié envoie les événements cachés (par exemple, "le fichier X a été ouvert").
- Assure-toi que tout fonctionne avec des permissions utilisateur normales et root.

#### **Jour 13 : Rapport initial**
- Rédige une première version du rapport avec les sections suivantes :
  - Présentation générale du projet.
  - Explication du fonctionnement de LD_PRELOAD et de ton hook.
  - Implémentation du serveur TCP.

#### **Jour 14 : Révisions finales**
- Relis ton code pour vérifier qu’il est bien structuré (évite les gros fichiers `.c` uniques).
- Corrige les bugs mineurs.
- Finalise le rapport avec une explication claire de la méthodologie et des outils utilisés.

---

### **Planning hebdomadaire simplifié :**

| Jour       | Tâches principales                                  | Temps estimé |
|------------|-----------------------------------------------------|--------------|
| **Semaine 1** | **LD_PRELOAD** : mise en place et hooks de base      | 15-20h       |
| **Semaine 2** | **Command & Control** : serveur TCP multithread      | 15-20h       |
| **Semaine 3** | **Furtivité et finalisation** : cacher un fichier, tests, rapport | 15-20h       |


## **Ambre**
 - Montage du conteneur docker pour les test sécurisé avec le dossier partagé :
    (travail)
    `docker run -it --name partiel_langage_c -v /c/Users/ambre/perso-esgi/PartielLangageC/PartielLangageC:/workspace ubuntu:latest`

 - Une fois dans le conteneur
    `cd /workspace`
    `apt update && apt install -y build-essential`

 - Modification du code directement sur l'ordi et exécution sur le docker

 - Stoper conteneur : `docker stop partiel_langage_c`

 - Reprendre le travail : `docker start -ai partiel_langage_c`
