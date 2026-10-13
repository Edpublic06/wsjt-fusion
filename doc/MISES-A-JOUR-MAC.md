# Maintenir WSJT-Z sur Mac

## Références conservées

- Dépôt personnel : https://github.com/Edpublic06/wsjt-z
- origin : dépôt personnel.
- upstream : https://github.com/sq9fve/wsjt-z.git
- macos-2.0.19 : sources de référence et documentation initiale.
- macos-build-maintenance : script de reconstruction et comptes rendus.
- Publication conservée : v2.0.19-macos-m1.
- Application habituelle : /Applications/wsjtx.app.
- Script : doc/build-macos-preserve.sh.
- Reconstruction vérifiée : doc/macos-build-records/macos.9Ielpt/.

Le premier essai confirme le démarrage et la reprise des réglages.
Les tests détaillés CAT, PTT, audio et automatismes restent à consigner.

## 1. Vérifier les nouveautés de WSJT-Z

Depuis le dépôt local :

git status
git fetch upstream
git log --oneline v2.0.19..upstream/master

Ne pas commencer une mise à jour avec des modifications locales non sauvegardées.
git fetch récupère les références sans fusionner les changements.

Lors de la vérification du 13 octobre 2026, aucun nouveau commit
n’était présent sur upstream/master depuis v2.0.19.

## 2. Préparer un candidat séparé

Choisir le tag ou commit exact après lecture des notes de version.
Créer un nouveau dossier et une branche de test avec git worktree.

Exemple : remplacer REF_VERIFIEE par la référence réellement choisie,
et adapter les noms de branche et de dossier avant exécution :

git worktree add -b macos-candidat-01 ../WSJT-Z-candidat-01 REF_VERIFIEE

Ne pas utiliser un nom de branche ou un dossier déjà existant.
Ne pas fusionner directement dans macos-2.0.19.

## 3. Comparer les sources

Dans le dossier candidat :

git log --oneline v2.0.19..HEAD
git diff --stat v2.0.19 HEAD
git diff v2.0.19 HEAD -- doc/build-osx.sh CMakeLists.txt Darwin

Examiner les changements complets si nécessaire.
Comparer les exigences macOS, Qt, compilateurs, Hamlib et autres bibliothèques.

## 4. Réutiliser les adaptations Mac

Les adaptations actuellement documentées concernent :
- Les exclusions de fichiers générés dans .gitignore.
- Le script doc/build-macos-preserve.sh.
- La documentation de compilation et de tests.

Aucune correction locale du code applicatif n’a été documentée.

Le candidat créé depuis une référence amont peut ne pas contenir
ces adaptations. Les examiner puis les importer ou les adapter
explicitement dans le candidat, sans remplacer des fichiers existants
sans autorisation.

Pour chaque future correction applicative :
- Conserver si le problème persiste et si la correction reste adaptée.
- Adapter si le code amont a changé.
- Abandonner si l’amont a déjà corrigé le problème.
- Vérifier le comportement, même si un patch s’applique sans conflit.

## 5. Compiler sans remplacer l’application habituelle

Après vérification et adaptation du script dans le candidat :

bash doc/build-macos-preserve.sh

Le script crée un dossier build/macos.XXXXXX et y place le résultat.
Il ne lance ni brew update ni brew install.

Les dépendances Homebrew restent partagées : le dossier candidat
ne les isole pas et ne fige pas leurs versions.
Toute modification des dépendances doit être examinée et autorisée.

Conserver le journal Terminal, les versions, le commit, les différences
locales et la version exacte du script exécuté.
Un commit seul ne décrit pas les fichiers non suivis ni toutes
les modifications présentes lors de la construction.

## 6. Sauvegarder avant le lancement

Fermer WSJT-Z et WSJT-X.
Créer une nouvelle sauvegarde privée, sans écraser une sauvegarde précédente.

Emplacements repérés sur cette machine :
- ~/Library/Preferences/org.k1jt.wsjtx.plist
- ~/Library/Preferences/WSJT-X.ini
- ~/Library/Preferences/org.k1jt.wsjtx.WSJT-X.plist
- ~/Library/Application Support/WSJT-X
- ~/Library/Application Support/ft2
- /Applications/wsjtx.app

Ces emplacements ne garantissent pas l’exhaustivité pour une future version
ou pour un autre profil. Vérifier les chemins réellement utilisés.

Ne pas publier les préférences, identifiants ou journaux de contacts.

## 7. Tester le candidat

Ne pas lancer les deux applications simultanément.
Deux bundles différents peuvent partager les mêmes données.

Commencer avec Auto CQ, Auto Call et Enable Tx désactivés.
Vérifier successivement :
1. Version et révision dans À propos.
2. Reprise des réglages et stabilité.
3. Réception audio et décodage.
4. CAT : fréquence et mode lus et commandés correctement.
5. PTT : passage émission/réception et relâchement.
6. Audio émission lors d’un essai bref et maîtrisé.
7. QSO manuel et journalisation.
8. Automatismes, chacun séparément, avec arrêt vérifié.
9. Applications associées réellement utilisées.
10. Persistance des réglages après redémarrage.

Consigner le matériel, les paramètres et les résultats.
Surveiller le poste pendant les essais d’émission automatique.

## 8. Revenir à la version précédente

Arrêter l’émission et les automatismes, puis fermer le candidat.
Relancer :

open /Applications/wsjtx.app

Vérifier les réglages et le fonctionnement.
Si le candidat a modifié les données communes, une restauration
peut être nécessaire : fermer les applications, sauvegarder l’état
en échec puis demander l’autorisation avant de remplacer des données.

Ne pas fusionner automatiquement deux configurations ou journaux ADIF.

## 9. Publier uniquement après validation

Conserver la publication précédente.
Créer un nouveau tag et une nouvelle publication pour chaque version.
Construire un nouveau .dmg depuis le bundle candidat validé.
Ne pas écraser le .dmg précédent.

Documenter :
- Source amont et commit exact.
- Corrections locales.
- Script et environnement de compilation.
- Machine et macOS testés.
- Résultats et limites des tests.
- Procédure de retour arrière.

## 10. Cas distinct : intégrer une nouvelle version de WSJT-X

Une évolution de WSJT-X n’est pas automatiquement une mise à jour de WSJT-Z.
La version actuelle de ce projet repose sur WSJT-X 3.0.0.

Le numéro 3.3.0-beta1 a été mentionné par l’utilisateur ; son origine,
son code source et sa compatibilité n’ont pas été vérifiés dans ce travail.

Avant une intégration directe :
1. Identifier et vérifier la source officielle et la référence exacte.
2. Identifier la base WSJT-X utilisée par WSJT-Z.
3. Comparer les changements amont et les fonctions spécifiques de WSJT-Z.
4. Créer une branche et un dossier dédiés à cette intégration.
5. Résoudre les conflits sans perdre les automatismes et autres extensions.
6. Adapter les dépendances et la construction Mac.
7. Refaire les tests fonctionnels et radio.

Aucune commande de fusion automatique avec WSJT-X n’est validée ici.
Cette intégration exige une analyse du code ; changer le numéro de version
ne suffit pas.
