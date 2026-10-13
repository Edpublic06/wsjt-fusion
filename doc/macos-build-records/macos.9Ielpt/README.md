# Reconstruction Mac de WSJT-Z 2.0.19

## Références
- Branche : macos-build-maintenance
- Commit lors de la compilation : 80ac1bd0a009d6ebfc7b4bdd0f5655a9938cad34
- Script utilisé : doc/build-macos-preserve.sh
- Script non suivi par Git au moment de la compilation, puis enregistré dans le commit 31592db.
- Aucun changement du code applicatif documenté.

## Environnement constaté dans le journal
- Architecture : arm64
- Compilateurs C/C++ : AppleClang 17.0.0.17000013
- Fortran : GNU Fortran 16.2.0
- Chemin Fortran : /opt/homebrew/bin/gfortran
- Qt : 5.15.19
- Hamlib : 4.7.2
- Inventaire Homebrew complet : dependances.txt
- Cet inventaire liste les paquets installés ; il ne prouve pas qu'ils sont tous utilisés.

## Commande exécutée
bash doc/build-macos-preserve.sh

## Résultat confirmé
- Construction et préparation du bundle terminées.
- Vérification des 20 exécutables réussie.
- Signature : codesign result: 0.
- Application produite :
  build/macos.9Ielpt/stage/wsjtx.app
- Application installée dans /Applications/wsjtx.app non remplacée.
- Aucun test de lancement ou de fonctionnement radio de cette reconstruction encore rapporté.

## Comportement du script
- Aucun brew update ou brew install.
- Aucune installation automatique de Homebrew ou des outils Apple.
- Nouveau dossier de construction à chaque exécution.
- Enregistrement du commit, de l'état Git et des paquets installés.
- Les dépendances restent partagées avec les autres constructions ; elles ne sont pas figées.

## Avertissements observés
- Avertissements de compatibilité et de politique CMake.
- OpenMP C/C++ non trouvé ; OpenMP Fortran trouvé.
- Ces messages n'ont pas empêché la construction.
- Le message "replacing existing signature" est accompagné d'un résultat de signature égal à 0.

## Pour une future version
Comparer son script et ses exigences de dépendances avant de réutiliser ce script.
Construire et tester un candidat séparé, sans remplacer l'application fonctionnelle.
Sauvegarder la configuration avant le lancement : deux bundles peuvent partager les mêmes données.
