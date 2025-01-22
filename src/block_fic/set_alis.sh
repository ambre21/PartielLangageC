#!/bin/bash

# Ajouter l'alias pour la commande 'cat' pour qu'elle utilise block_fic.so
alias cat='LD_PRELOAD=/home/victim/Bureau/test_C/block_fic.so /bin/cat'

# Afficher un message pour indiquer que l'alias a été ajouté
echo "Alias 'cat' avec block_fic.so ajouté avec succès."
