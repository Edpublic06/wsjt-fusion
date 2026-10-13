# WSJT-Fusion — registre des sources et adaptations

État au 13 octobre 2026. Le registre distingue les sources présentes dans
le socle des sources téléchargées pour l'intégration en cours.

| Source | Référence immuable | État |
| --- | --- | --- |
| WSJT-Z original | `sq9fve/wsjt-z`, `v2.0.19`, `b3a06e89ae0174c51c8595d99e40ca179738eb0a` | Socle applicatif présent. |
| Maintenance macOS Edpublic06 | `Edpublic06/wsjt-z`, `macos-build-maintenance`, `e850c68` | Documentation et procédure présentes ; aucun changement applicatif par rapport au tag Z. |
| WSJT-X | `WSJTX/wsjtx`, `v3.3.0-beta1`, `57ba9337fa020065dc0ceee2c4ef85e433ea32d4` | Cœur JTTY importé et testé ; raccordement interface/audio et moteur général encore à réaliser. |
| WS / Improved | `ws-3.2.1_261010.tgz` | Récepteur FT2 intégré dans un module privé ; compilation macOS et premiers tests synthétiques réussis. |

Archive WS officielle :
https://sourceforge.net/projects/wsjt-x-improved/files/WS_v3.2.1/Source%20code/ws-3.2.1_261010.tgz/download

SHA-256 de l'archive téléchargée :
`b343f440baec95c6cf6eaf3766802c4c6533a2b3129fbecf23d9a096daf2a0d5`.
Les sources applicatives sont dans l'archive imbriquée `src/ws.tgz`.

## FT2 du socle

- Introduction déclarée depuis Improved : `ec2a6037502bb177e93996d2224c97866b146a5b`.
- Correction de la période à 3,75 s : `c467f6653c5f79d0122fcac411b3caf27f223e2e`.
- Réception active : `lib/decoder.f90`, branche `nmode=52`, moteur FT4 et
  callback `ft2_decoded`.
- Émission et commandes : `widgets/mainwindow.*`, `Modulator`, modes,
  fréquences, affichage et journalisation associés.
- `lib/ft2` contient également un ancien expérimental différent. Son contenu
  ne suffit pas à identifier l'implémentation utilisée par l'application.
- Révision Improved initiale et révision JTDX précise : non établies.

## Règles pour les imports effectifs

Premier lot FT2, modifié le 13 octobre 2026 :
`wsjtz/decoding/ft2/upstream/` contient les sources dérivées avec attribution,
date et noms privés. La liste et les empreintes originales sont dans
`wsjtz/decoding/ft2/source-manifest.json`. Raccordements modifiés :
`lib/decoder.f90`, `lib/jt9.f90`, `widgets/mainwindow.cpp`, `CMakeLists.txt`.
Voir `wsjtz/decoding/ft2/README.md` pour les adaptations et validations.

Extension personnelle Auto Call, 13 octobre 2026 :
`wsjtz/automation/IncomingCallPolicy.hpp` et raccordements dans
`widgets/mainwindow.{cpp,h,ui}`. Option désactivée par défaut ; abandon d'un
appel sans réponse au profit d'un appel direct éligible, protection du nouveau
QSO et reprise du cycle normal Auto Call. Cette fonction est propre à cette
branche, et n'est pas attribuée à un projet amont. Compilation macOS et test
de décision réussis ; essai radio à réaliser.

Premier lot JTTY, 13 octobre 2026 : sources conservées sans modification,
chemins et empreintes dans `wsjtz/jtty/source-manifest.json`. Les nouveaux
fichiers de raccordement CMake sont propres à cette branche. Six tests amont
de codec/réception réussis ; interface, émission et audio de l'application
non raccordés à ce stade. Voir `wsjtz/jtty/README.md`.

Pour chaque lot, enregistrer les fichiers/symboles repris, la référence source,
la date de modification, les adaptations personnelles, le commit d'intégration,
les notices conservées et les validations réalisées. Préserver les en-têtes
et les fichiers de licence tiers. Ne pas déclarer intégré un lot seulement
téléchargé, ni validé un lot seulement compilé.

File de réponses CQ, 13 octobre 2026 : extension personnelle dans
`wsjtz/automation/CqReplyQueue.hpp`, avec raccordements réception, fin du
QSO, délai d'appel et sélection manuelle dans `widgets/mainwindow.*`.
Toujours active pour la réponse automatique aux CQ des modes pris en charge.
Les sources amont ne sont pas créditées comme auteurs de cette nouvelle
fonction. Compilation macOS et test de file réussis ; essai radio à réaliser.

Extension de la file aux appels directs reçus pendant un QSO déjà en cours,
13 octobre 2026 : association du correspondant courant à la session de file,
attente jusqu'à sa journalisation, dédoublonnage conservé. L'option Auto Call
reste applicable aux tentatives sans réponse. Compilation macOS et deux
régressions d'automatisation réussies ; validation radio à effectuer.

Réglages des extensions personnelles déplacés vers WSJT-Z / Misc,
13 octobre 2026 : priorité aux appelants et limite d'émissions sans réponse
pour chaque station sortie de la file. Valeur par défaut « WD Timer ».
Configuration acceptée/annulée selon le fonctionnement existant du dialogue.
Compilation macOS, vérification de l'emplacement XML et régressions réussies.

Nom final du champ : WD Counter, dans WSJT-Z / Misc. Les paramètres sont
isolés sous Configuration/Edpublic06/PreferIncomingCalls et
Configuration/Edpublic06/WDCounter. Test de persistance effectué sur un
fichier temporaire : conservation des clés et valeurs WD et d'un autre MOD,
avec exactement deux nouvelles clés. Cela ne constitue pas un test de tous
les MOD disponibles. Aucune conversion globale de paramètres n'est ajoutée.

WD Counter affiche finalement 0 (valeur initiale). À 0, le chemin WD existant
est conservé, avec Custom WD Timer du mode lorsqu'il est activé. Une valeur
positive utilise le compteur par station sortie de la file.

Le dépôt a été renommé `Edpublic06/wsjt-fusion`. Les anciennes références `Edpublic06/wsjt-z` ci-dessus désignent son historique antérieur au changement de nom. La version applicative Fusion 1.0.0-beta1 est indépendante des versions amont.
