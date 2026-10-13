# WSJT-Fusion — registre des sources et adaptations

État au 13 octobre 2026. Le registre distingue les sources présentes dans
le socle des sources téléchargées pour l'intégration en cours.

## Tableau des versions par fonction

**Version de l’application : WSJT-Fusion 1.0.0-beta1.** Ce numéro appartient au fork ; il ne décrit pas la version de tous les décodeurs. État au 13 octobre 2026.

| Fonction / composant | Projet d’origine | Version de référence | Révision / preuve | Ce qui est effectivement présent dans Fusion |
| --- | --- | --- | --- | --- |
| Socle applicatif, interface, filtres, Auto CQ / Auto Call historiques | WSJT-Z, SQ9FVE et contributeurs | **2.0.19** | [Tag original](https://github.com/sq9fve/wsjt-z/tree/v2.0.19), commit `b3a06e89ae0174c51c8595d99e40ca179738eb0a` | Socle conservé, avec adaptations et fonctions personnelles ; Fusion n’est pas une publication officielle de WSJT-Z. |
| Moteur général hérité | WSJT-X via le socle WSJT-Z | **3.0.0** (base déclarée) | [`Versions.cmake`](Versions.cmake) ; snapshot du socle ci-dessus | Base encore utilisée. Elle ne signifie pas que chaque fichier est identique au tag WSJT-X 3.0.0 ; WSJT-Z contient ses propres modifications. |
| FT8, décodage et multithread | WSJT-X + adaptations WSJT-Z, contributions JTDX | Base WSJT-X **3.0.0** ; WSJT-Z **2.0.19** ; version JTDX **non établie** | [`lib/ft8`](lib/ft8), [`lib/decoder.f90`](lib/decoder.f90), historique du socle | Moteur hérité avec améliorations spécifiques. **Migration complète au FT8 de WSJT-X 3.3.0-beta1 non terminée.** Ne pas attribuer une version JTDX précise sans preuve. |
| FT4 standard | WSJT-X via WSJT-Z | Base déclarée **3.0.0**, socle Z **2.0.19** | [`lib/ft4_decode.f90`](lib/ft4_decode.f90) et historique du socle | FT4 standard conservé ; l’import récent pour FT2 est isolé et ne remplace pas ce décodeur. |
| FT2, récepteur mis à jour | WS / WSJT-X Improved et contributeurs | **WS 3.2.1, build 261010** | Archive `ws-3.2.1_261010.tgz`, [manifest des fichiers et empreintes](wsjtz/decoding/ft2/source-manifest.json) ; import Fusion [`095e907`](https://github.com/Edpublic06/wsjt-fusion/commit/095e907) | Récepteur adapté depuis les sources FT4 utilisées pour FT2, dans un module privé. Compilation et tests synthétiques réussis ; cela ne certifie pas une reprise intégrale de toutes les fonctions de WS. |
| FT2, émission et fonctionnement applicatif hérités | WSJT-Z, port initial déclaré depuis Improved | Socle Z **2.0.19** ; révision Improved initiale **non établie** | Port `ec2a6037502bb177e93996d2224c97866b146a5b` ; période 3,75 s `c467f6653c5f79d0122fcac411b3caf27f223e2e` | Émission, sélection du mode, interface et fonctionnement hérités, avec raccordements Fusion. L’ancien répertoire expérimental `lib/ft2` ne représente pas à lui seul le FT2 actif. |
| JTTY, cœur codec / réception | WSJT-X et contributeurs | **3.3.0-beta1** | [Tag amont](https://github.com/WSJTX/wsjtx/tree/v3.3.0-beta1), commit `57ba9337fa020065dc0ceee2c4ef85e433ea32d4` ; [manifest](wsjtz/jtty/source-manifest.json) | Cœur importé, six tests amont réussis. **Interface, audio et émission de l’application non raccordés : mode utilisateur pas encore disponible.** |
| Nouveau moteur général ciblé | WSJT-X | **3.3.0-beta1** | Même tag et commit amont ; [plan d’intégration](doc/PLAN-3.3.0-beta1.md) | Référence de travail. **Intégration générale encore en cours**, distincte du lot JTTY déjà importé. |
| Autres modes hérités : JT4 / JT9 / JT65, Q65, MSK144, WSPR, Echo, FreqCal | WSJT-X via WSJT-Z | Base déclarée **3.0.0**, socle Z **2.0.19** | Historique du socle ; inventaire précis par fichier à compléter | Fonctions héritées. Aucun port complet de ces modes depuis 3.3.0-beta1 n’est déclaré. |
| Priorité aux appelants, file d’attente dédoublonnée, WD Counter | WSJT-Fusion, F6FDY | **1.0.0-beta1** | [`wsjtz/automation`](wsjtz/automation), historique Fusion | Extensions propres au fork. Corrections récentes : protection du premier appelant [`8a28ecc`](https://github.com/Edpublic06/wsjt-fusion/commit/8a28ecc), respect du filtre des appelants [`b34f533`](https://github.com/Edpublic06/wsjt-fusion/commit/b34f533). Tests automatisés réussis ; essais radio en cours. |

Les lignes « base déclarée » décrivent la provenance du socle, pas une comparaison complète de chaque fichier avec un tag amont. Les versions inconnues restent explicitement inconnues. Les manifests permettent une traçabilité plus précise que le seul numéro affiché.

## Références immuables des projets

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
pour chaque station sortie de la file. Valeur par défaut 0, utilisant le Custom WD Timer du mode lorsqu’il est activé.
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
