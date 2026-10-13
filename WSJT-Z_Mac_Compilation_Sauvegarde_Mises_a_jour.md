# WSJT-Z sur Mac — compilation, sauvegarde, restauration et mises à jour

Document établi le 9 octobre 2026 à partir de la conversation uniquement, puis complété par la validation de l’utilisateur.

## 1. Portée et règles d’utilisation

Ce document distingue :

- **Confirmé** : information ou résultat visible dans la conversation, notamment les sorties Terminal fournies par l’utilisateur.
- **Consulté** : contenu du script du projet lu pendant la conversation ; cela décrit son fonctionnement, sans prouver la version de chaque outil réellement installé.
- **Proposé** : procédure à exécuter ultérieurement, non réalisée ni validée dans cette conversation.
- **Inconnu** : information absente des preuves disponibles.

Les procédures futures ne doivent supprimer, remplacer ni modifier un fichier existant sans autorisation. Elles utilisent de nouveaux dossiers. Aucun lancement, test, sauvegarde ou changement système décrit comme « proposé » n’a été exécuté pour rédiger ce document.

L’utilisateur a ensuite confirmé : « oui toute cette procédure fonctionne ». La compilation, l’utilisation de l’application et la procédure de création et d’ouverture du `.dmg` sont donc validées par son retour. Aucun compte rendu détaillé ne précise séparément les résultats du CAT, du PTT, du décodage ou des automatismes ; leur grille de tests reste disponible pour les futures versions. Cette confirmation ne constitue pas un relevé des versions des dépendances ni une preuve d’exécution de chaque procédure future de sauvegarde ou de restauration.

## 2. État connu de la machine et du logiciel

| Élément | Information vérifiable | Statut |
|---|---|---|
| Ordinateur | MacBook Pro 13 pouces, Apple Silicon, 2020 | Confirmé par capture |
| Mémoire | 16 Go | Confirmé par capture |
| macOS | 15.8.1, build 24H212 | Confirmé par capture |
| Application antérieure | Version désignée par l’utilisateur comme « wsjtx-z 2.6.A » | Confirmé ; emplacement et identification complète inconnus |
| Dépôt utilisé | `https://github.com/sq9fve/wsjt-z.git` | Confirmé |
| Version téléchargée | Tag `v2.0.19` | Confirmé |
| Version affichée dans « À propos » | WSJT-Z `2.0.19 - b3a06e` | Confirmé par capture fournie par l’utilisateur |
| Commit téléchargé | `b3a06e89ae0174c51c8595d99e40ca179738eb0a` | Confirmé par Git |
| Base annoncée pour WSJT-Z 2.0.19 | WSJT-X 3.0.0 | Consulté dans les notes de version |
| Dossier du code | `/Users/eddiefillia/Applications/WSJT-Z-build/wsjt-z` | Confirmé |
| Application construite | `/Users/eddiefillia/Applications/WSJT-Z-build/wsjt-z/stage/wsjtx.app` | Confirmé |
| Application fonctionnelle installée | `/Applications/wsjtx.app` | Emplacement fourni et confirmé par l’utilisateur |
| Date de compilation | 3 octobre 2026 | Confirmé par l’utilisateur ; heure non précisée |
| Application construite fonctionnelle | Fonctionnement confirmé globalement par l’utilisateur | Confirmé par retour utilisateur ; détails des tests non consignés |

La publication 2.0.19 consultée contenait deux installateurs Windows et deux archives de sources, sans application Mac prête à installer. Le script Mac consulté prévoit `arm64` et `x86_64` avec une cible de déploiement macOS 11.0. Ces constats concernent la version consultée, pas les futures publications.

## 3. Historique des opérations

### 3.1 Commandes effectivement rapportées

Les commandes ci-dessous reproduisent les étapes confirmées par la sortie Terminal de l’utilisateur. Le lien Git est écrit sous sa forme brute, directement utilisable dans Terminal.

```bash
xcode-select --install
mkdir -p ~/Applications/WSJT-Z-build
cd ~/Applications/WSJT-Z-build
git clone --branch v2.0.19 --depth 1 https://github.com/sq9fve/wsjt-z.git
cd wsjt-z
```

Apple a répondu :

```text
Command line tools are already installed.
```

Git a téléchargé les sources puis sélectionné le commit indiqué en section 2. Le message `detached HEAD` est attendu lors de la sélection d’un tag : il ne constitue pas une erreur de compilation.

### 3.2 Compilation

La commande transmise était :

```bash
bash doc/build-osx.sh
```

La ligne de lancement n’apparaît pas dans l’extrait final fourni, mais sa sortie atteste la fin de construction et de préparation de l’application :

```text
verified='1'
Verified 20 executable files in '/Users/eddiefillia/Applications/WSJT-Z-build/wsjt-z/stage/wsjtx.app'
fixup_bundle: done
codesign result: 0
Done. App bundle should be under: /Users/eddiefillia/Applications/WSJT-Z-build/wsjt-z/stage/wsjtx.app
```

Le message `replacing existing signature`, présenté sous une ligne nommée `codesign error`, n’est pas une preuve d’échec : le résultat rapporté de la commande est `0`. Cela n’atteste pas une notarisation Apple, qui n’a pas été documentée.

### 3.3 Lancement et validation par l’utilisateur

Les étapes suivantes ont été suggérées avant la confirmation globale de fonctionnement :

- Sauvegarde des données de l’ancienne application.
- Fermeture de l’ancienne application et lancement de la nouvelle.
- Vérification de la version dans « À propos ».
- Vérification de la reprise des réglages.
- Modification des limites de mémoire partagée, uniquement en cas d’erreur.

Commande de lancement transmise :

```bash
open ~/Applications/WSJT-Z-build/wsjt-z/stage/wsjtx.app
```

L’utilisateur confirme ensuite que la procédure fonctionne. L’exécution éventuelle des commandes conditionnelles de mémoire partagée et les détails de la sauvegarde initiale ne sont pas précisés ; ne pas les présenter comme des opérations attestées individuellement.

L’utilisateur précise que la compilation a été effectuée le **3 octobre 2026**. La capture de la fenêtre « À propos » affiche **WSJT-Z — Version 2.0.19 - b3a06e**. Cette révision abrégée correspond au début du commit téléchargé `b3a06e89ae0174c51c8595d99e40ca179738eb0a` et confirme l’identification de l’application lancée.

L’utilisateur a ensuite fourni et confirmé **`/Applications/wsjtx.app`** comme emplacement de la version fonctionnelle installée. Le chemin dans `stage` reste celui de la sortie de compilation ; pour sauvegarder l’exemplaire effectivement utilisé, prendre le bundle installé dans `/Applications/wsjtx.app`. La méthode de copie vers Applications et sa date ne sont pas documentées.

Commande de lancement de l’exemplaire installé, pour utilisation ultérieure :

```bash
open /Applications/wsjtx.app
```

## 4. Dépendances et versions

### 4.1 Dépendances prévues par le script consulté

Le script s’appuie sur les outils de ligne de commande Apple et Homebrew. Il utilise la commande suivante :

```bash
brew install cmake ninja pkg-config gcc boost fftw hamlib libusb portaudio asciidoctor ccache qt@5
```

| Composant | Fonction dans la construction | Version connue |
|---|---|---|
| Outils de ligne de commande Apple | Outils de développement | Installés ; version inconnue |
| Homebrew | Installation des dépendances | Inconnue |
| CMake | Configuration et installation | Inconnue |
| Ninja | Exécution de la construction | Inconnue |
| pkg-config | Recherche des bibliothèques | Inconnue |
| GCC / gfortran | Compilateur Fortran | Inconnue |
| Boost | Bibliothèques C++ | Inconnue |
| FFTW | Calculs de transformées de Fourier | Inconnue |
| Hamlib | Interface de commande radio | Inconnue |
| libusb | Accès USB | Inconnue |
| PortAudio | Interface audio | Inconnue |
| Asciidoctor | Outillage documentaire | Inconnue |
| ccache | Cache de compilation | Inconnue |
| Qt | Interface graphique ; formule `qt@5` | Famille Qt 5 ; version exacte inconnue |

**Aucune version exacte des dépendances ne peut être reconstituée à partir des extraits reçus.** Le succès final ne prouve pas non plus que chaque paquet a été nouvellement installé : certains pouvaient être présents.

### 4.2 Paramètres du script consulté

Le script détecte l’architecture, recherche Qt 5 et gfortran, puis configure et construit le projet. Voici la commande consultée ; elle ne peut pas être copiée seule sans les variables préparées par le script :

```bash
cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_OSX_ARCHITECTURES="$CMAKE_ARCH" \
  -DCMAKE_OSX_DEPLOYMENT_TARGET="$CMAKE_DEPLOY_TARGET" \
  -DCMAKE_Fortran_COMPILER="$GFORTRAN_BIN" \
  -DCMAKE_PREFIX_PATH="$QT5_PREFIX;$(brew --prefix)" \
  -DQt5_DIR="$QT5_PREFIX/lib/cmake/Qt5" \
  -DFFTW3_ROOT_DIR="$(brew --prefix fftw)" \
  -DHamlib_ROOT="$(brew --prefix hamlib)" \
  -DUsb_ROOT="$(brew --prefix libusb)" \
  -DWSJT_SKIP_MANPAGES=ON \
  -DWSJT_GENERATE_DOCS=OFF \
  -DCMAKE_INSTALL_PREFIX="$PWD/stage"
cmake --build build -j"$(sysctl -n hw.ncpu)"
cmake --install build
```

Sur la machine macOS Apple Silicon attendue, les valeurs prévues sont `CMAKE_ARCH=arm64` et `CMAKE_DEPLOY_TARGET=11.0`. Ne pas confondre cette cible minimale avec la version du Mac ayant effectué la construction.

### 4.3 Inventaire à effectuer ultérieurement

**Proposé.** Ces commandes lisent l’état actuel et écrivent uniquement dans un nouveau dossier. Cet inventaire prouvera l’état au moment de sa création ; il ne prouvera pas rétroactivement les versions utilisées lors de la compilation initiale.

```bash
mkdir -p "$HOME/Applications/WSJT-Z-archives"
wsjtz_inventory=$(mktemp -d "$HOME/Applications/WSJT-Z-archives/inventaire.XXXXXX")
sw_vers > "$wsjtz_inventory/macos.txt"
uname -m > "$wsjtz_inventory/architecture.txt"
xcode-select -p > "$wsjtz_inventory/apple-tools-path.txt"
pkgutil --pkg-info com.apple.pkg.CLTools_Executables > "$wsjtz_inventory/apple-tools-version.txt" 2>&1
brew --version > "$wsjtz_inventory/homebrew-version.txt"
brew list --versions > "$wsjtz_inventory/homebrew-packages.txt"
brew info --json=v2 cmake ninja pkg-config gcc boost fftw hamlib libusb portaudio asciidoctor ccache qt@5 > "$wsjtz_inventory/dependances.json"
cmake --version > "$wsjtz_inventory/cmake-version.txt"
ninja --version > "$wsjtz_inventory/ninja-version.txt"
"$(brew --prefix qt@5)/bin/qmake" -query > "$wsjtz_inventory/qt.txt"
printf 'Inventaire : %s\n' "$wsjtz_inventory"
```

Si `brew` n’est pas trouvé, interrompre cet inventaire et vérifier son emplacement ; le script initial prévoit `/opt/homebrew/bin/brew` ou `/usr/local/bin/brew`. Ne pas interpréter un fichier vide ou une erreur comme une version valide.

## 5. Modifications du code source

**Aucune correction locale du code source n’est documentée dans la conversation.** Le téléchargement du tag et la construction sont attestés ; aucune édition manuelle ou application de patch n’a été rapportée.

Les modifications macOS mentionnées dans les notes de version sont des changements du projet amont, pas des corrections effectuées par l’utilisateur. La construction et la signature du bundle ne constituent pas une correction locale du code source.

Il reste possible que des changements aient été faits hors conversation. Pour les détecter, la procédure future de sauvegarde enregistre `git status` et `git diff`. Les fichiers non suivis doivent aussi être examinés : ils ne sont pas inclus dans un simple patch Git.

## 6. Difficultés et solutions

| Observation | Diagnostic établi | Solution / statut |
|---|---|---|
| Pas d’installateur Mac dans la publication consultée | Sources disponibles, installateurs Windows seulement | Construction avec le script Mac ; réalisée |
| Outils Apple déjà installés | Message informatif | Aucune réinstallation nécessaire à cette étape |
| `detached HEAD` après clonage | Sélection du tag | Aucune correction nécessaire |
| `replacing existing signature` | Signature remplacée, résultat `0` | Pas d’échec établi |
| Limites de mémoire partagée basses | Avertissement du script | Ajustement proposé uniquement en cas d’erreur au lancement |

Valeurs rapportées par le script :

```text
kern.sysv.shmmax=52428800
kern.sysv.shmall=25600
```

Valeurs préconisées par ce script :

```text
kern.sysv.shmmax>=134217728
kern.sysv.shmall>=32768
```

**Proposé, non exécuté et soumis à autorisation puisqu’il modifie l’état du système.** Si l’application échoue avec `QSharedMemory` ou `system-imposed size restrictions` :

```bash
sudo sysctl -w kern.sysv.shmmax=134217728
sudo sysctl -w kern.sysv.shmall=32768
```

Ces réglages sont temporaires jusqu’au redémarrage. Aucun mécanisme de réglage permanent n’a été établi. Ne pas les appliquer automatiquement, ni supposer que les valeurs anciennes seront identiques plusieurs mois plus tard.

## 7. Sauvegarder l’état actuel

### 7.1 Identifier ce qui doit être conservé

**Proposé.** Avant de déclarer un exemplaire « fonctionnel », effectuer les tests de la section 10. Conserver séparément :

1. L’ancienne application 2.6.A, si elle fonctionne encore.
2. L’application 2.0.19 fonctionnelle installée dans `/Applications/wsjtx.app`, ainsi que la sortie de compilation conservée dans `stage`.
3. Les réglages, journaux de contacts et autres données utilisées par chaque application.
4. Le code complet et les éventuelles corrections locales.
5. Les versions des outils, les journaux de construction et les résultats de tests.

La conversation ne donne pas le chemin exact de la configuration, ni celui de l’ancienne application. Ne pas inventer ces chemins. Les identifier depuis l’application et le Finder. Le dossier des données proposé via le menu Fichier peut ne pas contenir tous les réglages ; cette exhaustivité doit être vérifiée avant toute migration.

Fermer les applications avant de copier les données pour éviter une sauvegarde partiellement écrite.

### 7.2 Archiver l’application construite et le code

**Proposé.** Chaque exécution crée un dossier neuf ; aucune archive précédente n’est remplacée. Arrêter en cas d’erreur et vérifier chaque archive avant de poursuivre.

```bash
mkdir -p "$HOME/Applications/WSJT-Z-archives"
wsjtz_backup=$(mktemp -d "$HOME/Applications/WSJT-Z-archives/sauvegarde-2.0.19.XXXXXX")
wsjtz_source="$HOME/Applications/WSJT-Z-build/wsjt-z"
ditto -c -k --sequesterRsrc --keepParent "$wsjtz_source/stage/wsjtx.app" "$wsjtz_backup/wsjtx-2.0.19.app.zip"
ditto -c -k --sequesterRsrc --keepParent /Applications/wsjtx.app "$wsjtz_backup/wsjtx-2.0.19-installee.app.zip"
ditto -c -k --sequesterRsrc --keepParent "$wsjtz_source" "$wsjtz_backup/sources-et-construction.zip"
git -C "$wsjtz_source" rev-parse HEAD > "$wsjtz_backup/commit.txt"
git -C "$wsjtz_source" status --short > "$wsjtz_backup/git-status.txt"
git -C "$wsjtz_source" diff --binary HEAD > "$wsjtz_backup/modifications-suivies.patch"
git -C "$wsjtz_source" ls-files --others --exclude-standard > "$wsjtz_backup/fichiers-non-suivis.txt"
shasum -a 256 "$wsjtz_backup/wsjtx-2.0.19.app.zip" "$wsjtz_backup/wsjtx-2.0.19-installee.app.zip" "$wsjtz_backup/sources-et-construction.zip" > "$wsjtz_backup/SHA256.txt"
printf 'Sauvegarde : %s\n' "$wsjtz_backup"
```

L’archive complète conserve aussi les fichiers présents que Git ignore. Elle peut être volumineuse. Ne pas considérer le dossier `build` comme réutilisable sans modification sur un autre emplacement : il peut contenir des chemins absolus. Une reconstruction doit partir d’un dossier neuf.

### 7.3 Ajouter les données et l’ancienne application

**Proposé.** Dans la même session Terminal, indiquer un dossier de données réellement identifié, sans guillemets ajoutés dans la réponse :

```bash
printf 'Chemin absolu du dossier de données confirmé : '
IFS= read -r wsjtz_data_path
if [ -d "$wsjtz_data_path" ]; then
  ditto -c -k --sequesterRsrc --keepParent "$wsjtz_data_path" "$wsjtz_backup/donnees.zip"
  printf '%s\n' "$wsjtz_data_path" > "$wsjtz_backup/emplacement-donnees.txt"
else
  printf 'Dossier introuvable : aucune sauvegarde des données effectuée.\n'
fi
```

Répéter la sauvegarde dans un nouveau dossier pour tout autre emplacement de configuration confirmé. Pour l’ancienne application, copier son bundle avec le Finder dans un dossier neuf, ou créer une archive distincte avec `ditto` après avoir confirmé son chemin. Ne pas substituer le chemin de la 2.0.19 à celui de la 2.6.A.

Ajouter à l’archive : ce document, l’inventaire de la section 4.3, les sorties Terminal disponibles, les captures des réglages et un compte rendu des tests. Noter les périphériques audio, le modèle du poste, les paramètres CAT/PTT, les ports utilisés et les applications associées. Ces détails sont actuellement inconnus.

Copier l’ensemble sur un second support, dans un nouveau dossier. Vérifier les sommes SHA-256 après transfert. Conserver la configuration à l’abri : elle peut contenir des identifiants.

## 8. Reproduire la version actuelle

### 8.1 Limite de reproductibilité

Le tag et le commit sont connus. Les versions exactes des outils ne le sont pas. Le script effectue `brew update` et installe les formules disponibles au moment de son exécution. Une reconstruction ultérieure peut donc utiliser des versions différentes et produire un résultat différent.

La sauvegarde du bundle permet de retrouver l’application construite. L’archive du code, les dépendances inventoriées et les journaux facilitent sa reconstruction ; ils ne garantissent pas une reconstruction identique octet pour octet ni la disponibilité future des anciennes dépendances.

### 8.2 Reconstruction dans un emplacement neuf

**Proposé.** Les commandes suivantes téléchargent les sources dans un nouveau dossier sans toucher au dépôt actuel :

```bash
mkdir -p "$HOME/Applications/WSJT-Z-reconstructions"
wsjtz_rebuild=$(mktemp -d "$HOME/Applications/WSJT-Z-reconstructions/version-2.0.19.XXXXXX")
git clone --branch v2.0.19 --depth 1 https://github.com/sq9fve/wsjt-z.git "$wsjtz_rebuild/source"
git -C "$wsjtz_rebuild/source" rev-parse HEAD
```

Vérifier que le résultat est exactement :

```text
b3a06e89ae0174c51c8595d99e40ca179738eb0a
```

Lire le script avant de l’exécuter :

```bash
cat "$wsjtz_rebuild/source/doc/build-osx.sh"
```

**Étape nécessitant une autorisation avant exécution :** le script peut installer Homebrew, mettre à jour ses métadonnées et installer ou modifier des dépendances existantes. L’isolation du dossier de code n’isole pas Homebrew. Après autorisation :

```bash
cd "$wsjtz_rebuild/source"
bash doc/build-osx.sh > "$wsjtz_rebuild/compilation.log" 2>&1
wsjtz_build_status=$?
printf 'Code de sortie : %s\n' "$wsjtz_build_status"
tail -n 40 "$wsjtz_rebuild/compilation.log"
```

Le journal est enregistré sans être affiché pendant la construction. Une demande interactive peut nécessiter de consulter la session ; ne pas lancer ce bloc sans surveillance. Si le code de sortie est différent de `0`, analyser le journal avant tout lancement. Si des corrections locales ont été identifiées dans la sauvegarde, elles doivent être examinées et appliquées uniquement à cette nouvelle copie, avec autorisation.

## 9. Préparer une future mise à jour

### 9.1 Sauvegarde et téléchargement

**Proposé.** Valider et archiver l’application utilisée avant toute mise à jour. Consulter la page des versions et relever le tag exact ; aucun futur numéro ne peut être présumé.

```bash
printf 'Tag exact de la nouvelle version : '
IFS= read -r wsjtz_new_tag
mkdir -p "$HOME/Applications/WSJT-Z-candidats"
wsjtz_candidate=$(mktemp -d "$HOME/Applications/WSJT-Z-candidats/candidat.XXXXXX")
git clone --branch "$wsjtz_new_tag" https://github.com/sq9fve/wsjt-z.git "$wsjtz_candidate/source"
git -C "$wsjtz_candidate/source" rev-parse HEAD > "$wsjtz_candidate/commit-candidat.txt"
git -C "$wsjtz_candidate/source" show-ref --tags > "$wsjtz_candidate/tags.txt"
```

Ce clonage n’utilise pas `--depth 1`, afin de disposer de l’historique pour la comparaison. En cas d’échec, ne pas poursuivre avec un dossier incomplet.

### 9.2 Comparer avec la version actuelle

```bash
git -C "$wsjtz_candidate/source" log --oneline v2.0.19..HEAD > "$wsjtz_candidate/commits-depuis-2.0.19.txt"
git -C "$wsjtz_candidate/source" diff --stat v2.0.19 HEAD > "$wsjtz_candidate/resume-changements.txt"
git -C "$wsjtz_candidate/source" diff v2.0.19 HEAD -- doc/build-osx.sh CMakeLists.txt Darwin > "$wsjtz_candidate/changements-compilation-macos.diff"
git -C "$wsjtz_candidate/source" diff v2.0.19 HEAD > "$wsjtz_candidate/changements-complets.diff"
cat "$wsjtz_candidate/source/doc/build-osx.sh"
```

Ces comparaisons portent sur les sources amont. Examiner aussi les notes de version, les changements du journal du projet et le patch local sauvegardé. Si `v2.0.19` n’est pas disponible dans ce clone, interrompre la comparaison et récupérer une copie de référence dans un dossier neuf ; ne pas conclure que le diff est vide.

### 9.3 Décider du sort des corrections antérieures

À ce jour, aucune correction locale n’est attestée. Pour toute correction découverte ou ajoutée plus tard, tenir ce registre :

| Correction | Problème et preuve | Fichiers | État dans l’amont | Décision | Test de validation |
|---|---|---|---|---|---|
| Aucune documentée actuellement | — | — | — | Ne rien appliquer automatiquement | — |

Décisions possibles :

- **Conserver** : le problème existe encore et la correction reste pertinente.
- **Adapter** : le problème persiste, mais le code ou les dépendances ont changé.
- **Abandonner** : l’amont a corrigé le problème ou rendu la correction inutile.

Un patch qui s’applique sans conflit n’est pas nécessairement encore correct. Tester le comportement qu’il était censé réparer. Toute modification doit être autorisée et limitée au candidat ; ne pas retoucher la copie fonctionnelle.

### 9.4 Adapter les dépendances et compiler

Comparer l’ancienne et la nouvelle liste de dépendances, la version majeure de Qt, les compilateurs, la cible macOS, les options CMake, la signature et l’emplacement de sortie. Ne pas réutiliser aveuglément la liste de la section 4.

Avant toute installation ou mise à jour des dépendances existantes, demander l’autorisation et conserver l’inventaire antérieur. Une mise à jour Homebrew peut aussi affecter l’application précédente ; garder son bundle archivé et vérifier qu’il peut être lancé indépendamment.

Après examen et autorisation, exécuter le script de la nouvelle version depuis le nouveau dossier et conserver le journal comme en section 8.2. Si le script a disparu ou changé de nom, rechercher les instructions de cette version ; aucun remplacement n’est établi ici.

Ne pas déplacer le candidat sur l’application fonctionnelle. Garder les deux bundles dans des emplacements différents, clairement identifiés, et noter leur chemin exact.

## 10. Tester le candidat sans compromettre la version utilisée

### 10.1 Limite importante de l’isolation

Deux bundles séparés peuvent utiliser les mêmes réglages et journaux. **Le dossier de construction neuf ne garantit pas l’isolation des données.** La conversation n’établit aucune option de profil séparé.

Avant le lancement, sauvegarder toutes les données et autoriser explicitement leur éventuelle modification. Pour une séparation plus forte, proposer un compte macOS de test distinct ; sa création nécessite une autorisation et sa mise en place n’a pas été vérifiée ici. Ne jamais faire fonctionner les deux versions simultanément sur le même poste.

### 10.2 Ordre de validation proposé

| Étape | Action | Critère de réussite |
|---|---|---|
| Identification | Vérifier « À propos » et le chemin du bundle lancé | Bonne version et bon candidat |
| Démarrage | Observer messages, fenêtres et stabilité | Aucun plantage ni erreur de mémoire |
| Réglages | Comparer avec les captures de référence | Indicatif, modes, ports et paramètres cohérents |
| Audio réception | Sélectionner l’entrée habituelle et recevoir une activité connue | Niveau visible et décodages cohérents |
| CAT | Utiliser le test CAT disponible, puis vérifier fréquence et mode | Lecture et commande conformes au poste |
| PTT | Tester brièvement avec une installation adaptée à l’émission | Passage émission/réception et relâchement correct |
| Audio émission | Tester une émission courte avec réglages maîtrisés | Sortie audio correcte et comportement radio attendu |
| QSO manuel | Réaliser un contact contrôlé | Séquence, arrêt et journal corrects |
| Automatismes | Tester séparément Auto CQ, Auto Call et les autres fonctions réellement utilisées | Sélection, enchaînement, arrêt et watchdog conformes |
| Intégrations | Tester les applications associées effectivement utilisées | Communication correcte |
| Redémarrage | Relancer puis vérifier les réglages conservés | Persistance correcte |

Laisser Auto CQ, Auto Call et Enable Tx désactivés au début. Vérifier la possibilité d’arrêter l’émission avant les tests automatiques. Surveiller le poste pendant les essais. Les libellés exacts des menus, le modèle du poste, l’interface, les ports et les réglages ne sont pas connus ici ; les relever dans la version réellement utilisée plutôt que les deviner.

Enregistrer chaque résultat avec la version, le commit, la date, le matériel, les paramètres et les éventuels symptômes. Ne déclarer la mise à jour fonctionnelle qu’après validation des fonctions nécessaires à l’usage réel.

## 11. Retour arrière et restauration

### 11.1 Retour immédiat

**Proposé.** En cas d’échec :

1. Arrêter les automatismes et l’émission, puis fermer le candidat.
2. Relancer le bundle précédent conservé à son emplacement initial.
3. Vérifier les réglages, le CAT, la réception et le relâchement du PTT.
4. Si les données communes ont été modifiées, arrêter la version précédente et suivre la restauration ci-dessous avec autorisation.

Un retour immédiat est fiable seulement si le précédent bundle est conservé et si ses données restent compatibles ou sont isolées. Sans identification et sauvegarde de la configuration, cette garantie ne peut pas être donnée.

### 11.2 Restaurer une archive sans écraser l’existant

**Proposé.** Indiquer le dossier de sauvegarde réel :

```bash
printf 'Chemin absolu du dossier de sauvegarde : '
IFS= read -r wsjtz_archive_path
shasum -a 256 -c "$wsjtz_archive_path/SHA256.txt"
```

Poursuivre uniquement si les sommes sont correctes. Le manifeste créé en section 7 utilise des chemins absolus : si l’archive a été déplacée, ces chemins doivent être pris en compte ; calculer les sommes des trois ZIP à leur nouvel emplacement et les comparer aux valeurs enregistrées, sans modifier le manifeste original.

```bash
mkdir -p "$HOME/Applications/WSJT-Z-restaurations"
wsjtz_restore=$(mktemp -d "$HOME/Applications/WSJT-Z-restaurations/restauration.XXXXXX")
ditto -x -k "$wsjtz_archive_path/wsjtx-2.0.19-installee.app.zip" "$wsjtz_restore/application"
printf 'Application extraite : %s/application/wsjtx.app\n' "$wsjtz_restore"
```

Pour les données, extraire également dans un dossier neuf :

```bash
ditto -x -k "$wsjtz_archive_path/donnees.zip" "$wsjtz_restore/donnees"
```

Cette extraction ne restaure pas encore les données à leur emplacement actif. Examiner le résultat et le chemin enregistré dans `emplacement-donnees.txt`. **Toute substitution de la configuration active nécessite une autorisation.** Avant substitution, fermer l’application et archiver l’état en échec dans un nouveau dossier. Ne pas fusionner automatiquement deux configurations ou deux journaux ADIF.

Pour reconstruire hors ligne, extraire `sources-et-construction.zip` dans un autre dossier neuf. Le code sera disponible, mais les dépendances ou ressources téléchargées pendant la construction peuvent encore manquer. Conserver le bundle fonctionnel est donc indispensable.

## 12. Informations à compléter après validation

- Version exacte de chaque dépendance et des outils Apple.
- Archiver la capture de « À propos » fournie : elle confirme `2.0.19 - b3a06e`. La compilation est datée du 3 octobre 2026 par l’utilisateur.
- Chemin du bundle de l’ancienne 2.6.A, s’il est encore conservé ; celui de la 2.0.19 fonctionnelle est confirmé : `/Applications/wsjtx.app`.
- Emplacements exhaustifs des réglages et des données.
- Modèle du poste, interface, ports CAT/PTT, périphériques audio et paramètres.
- Résultats des tests manuels et automatiques.
- Éventuelles corrections locales, avec patch et justification.
- Emplacement de la sauvegarde secondaire et vérification de restauration.

## 13. Références déjà consultées dans la conversation

Ces liens servent à retrouver les sources ; aucune recherche supplémentaire n’a été utilisée pour rédiger ce document.

- [Versions WSJT-Z](https://github.com/sq9fve/wsjt-z/releases)
- [Fichiers publiés pour 2.0.19](https://github.com/sq9fve/wsjt-z/releases/expanded_assets/v2.0.19)
- [Dépôt du projet](https://github.com/sq9fve/wsjt-z)
- [Script de construction du tag v2.0.19](https://github.com/sq9fve/wsjt-z/blob/v2.0.19/doc/build-osx.sh)

Les procédures proposées devront être relues à chaque nouvelle version. L’historique confirmé de ce document doit rester distinct des résultats futurs.

## 14. Créer une image disque `.dmg` de l’application compilée

**Procédure validée par l’utilisateur**, qui a confirmé « oui toute cette procédure fonctionne » après son ajout au document. Aucune sortie Terminal détaillée de création n’a été fournie ; la validation repose sur ce retour. Cette étape conserve ou permet de transférer l’application compilée dans une image disque ; elle ne remplace pas la sauvegarde de la configuration, des journaux, des sources et des dépendances.

### 14.1 Création

Fermer la nouvelle application. Vérifier qu’aucun fichier nommé `WSJT-Z-2.0.19-macOS-Apple-Silicon.dmg` n’existe déjà sur le Bureau. S’il existe, choisir un autre nom pour la nouvelle image ; ne pas supprimer ni remplacer le fichier existant.

Exécuter la commande fournie :

```bash
hdiutil create -volname "WSJT-Z 2.0.19" -srcfolder "$HOME/Applications/WSJT-Z-build/wsjt-z/stage/wsjtx.app" -format UDZO "$HOME/Desktop/WSJT-Z-2.0.19-macOS-Apple-Silicon.dmg"
```

- `-volname` définit le nom du volume visible à l’ouverture.
- `-srcfolder` désigne l’application compilée à inclure.
- `-format UDZO` demande une image disque compressée.
- Le dernier chemin désigne le nouveau fichier sur le Bureau.

### 14.2 Vérification et conservation

Ouvrir `WSJT-Z-2.0.19-macOS-Apple-Silicon.dmg` depuis le Bureau et vérifier que l’application est présente. La procédure actuelle est confirmée par l’utilisateur. Lors des prochaines créations, conserver également la sortie Terminal et le résultat de cette vérification.

Conserver le `.dmg` avec les sauvegardes de la section 7 et en copier un exemplaire sur le second support. Cette image ne prouve pas le fonctionnement radio de l’application et n’ajoute pas de notarisation Apple.

Pour une future version, adapter le numéro de version, le nom du volume, le chemin du bundle candidat validé et le nom du fichier de sortie. Toujours créer une nouvelle image, en conservant celle de la version précédente.
