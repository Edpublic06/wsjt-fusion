# WSJT-Z 2.0.19 Mac — relevé des dépendances

## Provenance et portée

Versions relevées sur le Mac de l’utilisateur avec la commande ci-dessous, puis fournies par capture Terminal dans la conversation. Le Terminal affiche une dernière connexion le 9 octobre à 11:07:05 ; ce message ne suffit pas à dater précisément l’exécution de la commande. La capture a été fournie dans la conversation le 13 octobre 2026.

Ces versions attestent les paquets installés au moment du relevé. Elles ne prouvent pas rétroactivement les versions utilisées pour la compilation réussie du 3 octobre 2026. Aucun paquet n’a été installé ou modifié pour établir ce document.

## Commande utilisée

```bash
/opt/homebrew/bin/brew list --versions cmake ninja pkg-config gcc boost fftw hamlib libusb portaudio asciidoctor ccache qt@5
```

La sortie montre aussi un téléchargement des données API Homebrew. La commande a donc consulté les paquets et rafraîchi des données de Homebrew ; aucun changement de version des paquets n’est rapporté.

## Versions rapportées

```text
asciidoctor 2.0.26
boost 1.92.0
ccache 4.14.1
cmake 4.4.3
fftw 3.3.11
gcc 16.2.0
hamlib 4.7.2
libusb 1.0.30
ninja 1.13.2
pkgconf 3.0.7
portaudio 19.7.0
qt@5 5.15.19
```

La commande demandait `pkg-config` ; Homebrew rapporte le paquet sous le nom `pkgconf`.

## Contexte de la compilation conservée

- MacBook Pro M1, 16 Go ; macOS 15.8.1 lors de la capture initiale.
- Compilation le 3 octobre 2026, selon confirmation de l’utilisateur.
- Sources WSJT-Z : tag `v2.0.19`, commit `b3a06e89ae0174c51c8595d99e40ca179738eb0a`.
- Version affichée : `2.0.19 - b3a06e`.
- Script utilisé : `doc/build-osx.sh`.
- Publication communautaire : [v2.0.19-macos-m1](https://github.com/Edpublic06/wsjt-z/releases/tag/v2.0.19-macos-m1).

## Utilisation pour les reconstructions et mises à jour

Conserver ce relevé avec la documentation de compilation. Avant une nouvelle construction, relever de nouveau les versions et comparer les résultats. La disponibilité future de ces versions via Homebrew n’est pas garantie ; cette liste ne constitue pas un mécanisme d’installation de versions figées.

Les versions exactes de Homebrew, des outils de ligne de commande Apple et du compilateur gfortran effectivement choisi lors de la compilation initiale restent non consignées. La version de la formule GCC ne suffit pas à prouver le chemin du compilateur utilisé par CMake.
