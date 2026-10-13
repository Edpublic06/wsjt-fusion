# WSJT-Z 3.3.0-beta1 — étude de faisabilité

Date : 13 octobre 2026.

## Sources examinées

- Dépôt : https://github.com/Edpublic06/wsjt-z
- Branche de départ : `macos-build-maintenance`, commit `e850c68`.
- Référence amont : https://github.com/WSJTX/wsjtx/releases/tag/v3.3.0-beta1
- Commit amont : `57ba9337fa020065dc0ceee2c4ef85e433ea32d4`.
- Nom et version cibles : **WSJT-Z 3.3.0-beta1**.

## Conclusion

L'intégration paraît techniquement réalisable à partir des sources examinées. Il s'agit d'un portage substantiel, comprenant le nouveau mode **JTTY**, et non d'un simple remplacement du répertoire `lib`. Cette conclusion est fondée sur une inspection des sources ; elle ne constitue pas une validation de compilation, de performances ou de fonctionnement radio.

La branche Z affiche actuellement `VERSION_Z=2.0.19` et une base `3.0.0` dans `Versions.cmake`. L'amont cible déclare `version=3.3.0`, `channel=BETA`, `prerelease=1` dans `release-state.txt`.

Après normalisation des fins de ligne, 216 fichiers communs de `lib` et 37 fichiers communs de `widgets` diffèrent. Ces différences comprennent aussi des changements amont et ne représentent pas exclusivement des modifications Z. Il faudra les attribuer à l'aide de l'historique avant de décider quoi porter.

## Modifications WSJT-Z à préserver

- Auto CQ, Auto Call, Auto Call Next, Pounce, files prioritaires et changements de bande.
- Filtres, alertes, états de stations travaillées et mises en évidence.
- QRZ, carte, PSK Reporter, journalisation et commandes UDP propres à Z.
- Décodage FT8 multithread configurable et suppression des doublons.
- Améliorations FT8 spécifiques réellement présentes dans les sources, à inventorier séparément des descriptions du README. Par exemple, une modification OSD est commentée dans `lib/ft8/ft8b.f90` après des faux décodages : ne pas la réactiver au seul motif qu'elle est annoncée dans le README.
- Mode FT2 et ses raccordements : les sources `lib/ft2` de Z sont absentes de cette référence amont.
- Adaptations macOS et procédure de reconstruction préservant les dépendances existantes.

## Points de raccordement critiques

1. **Moteur de décodage** : l'amont comporte `DecoderEngine.h` (ABI 7), `lib/decoder_engine.f90` et les nouveaux contrats par mode. Porter ensemble les interfaces C/C++ et Fortran, les sources et la construction.
2. **Communication avec le décodeur** : les nouveaux contrôles de disposition mémoire et de démarrage doivent rester cohérents. Les champs Z de `commons.h` et `lib/jt9com.f90`, notamment les options FT8, exigent une adaptation des deux côtés.
3. **FT8 spécifique** : le routage Z dans `lib/decoder.f90` est substantiel. Comparer le nouveau multithread amont et celui de Z ; préserver les fonctionnalités Z par un raccordement explicite, sans empiler aveuglément deux systèmes concurrents.
4. **Interface et audio** : `widgets/mainwindow.*`, `Detector`, `Modulator` et `Decoder/decodedtext.*` évoluent ensemble. Le remplacement complet de la fenêtre Z supprimerait ses extensions.
5. **Version et paquets** : unifier l'affichage et les noms de livrables autour de `3.3.0-beta1`, sans conserver le suffixe historique combinant base X et version Z. Conserver l'identité WSJT-Z et les attributions/licences amont.

## JTTY inclus dans la cible

Importer les sources `lib/jtty`, le décodeur et les contrôleurs JTTY, les flux et files audio, les composants d'interface `widgets/Jtty*`, les fichiers `mainwindow_jtty*.cpp`, les réglages, menus, fréquences, documentation et tests correspondants, ainsi que les dépendances de construction effectivement nécessaires.

JTTY reçoit et transmet en continu et peut commencer une émission à tout moment ; il ne suit pas les cycles synchronisés FT8/FT4. Les automatismes Z doivent donc être conservés pour leurs modes existants et rendus compatibles avec JTTY avant toute activation dans ce mode. Préserver particulièrement la propriété de la session d'émission, la file de messages, l'arrêt d'émission et le PTT. Examiner également les fonctions de macros, de concours et d'intégration N1MM/MMTTY disponibles dans l'amont, avec leurs restrictions de plateforme.

## Architecture modulaire requise

Exigence ajoutée : faciliter les mises à jour successives depuis WSJT-X en isolant, autant que possible, les fonctions propres à WSJT-Z. Cette organisation est une cible de réalisation ; elle n'est pas encore implémentée.

### Trois couches

| Couche | Responsabilité | Règle de maintenance |
| --- | --- | --- |
| Code amont WSJT-X | Décodeurs, codecs, transport audio et fonctions standard, dont JTTY | Conserver les chemins et le contenu amont autant que possible pour faciliter les comparaisons et reprises de commits. |
| Adaptation WSJT-Z | Traduction des demandes, résultats, capacités et événements entre l'amont et les extensions | Concentrer les dépendances aux interfaces amont dans un petit nombre de fichiers identifiés. |
| Extensions WSJT-Z | Automatismes, filtres, files prioritaires, alertes, intégrations et particularités FT8/FT2 | Regrouper dans des modules dédiés ; ne pas dépendre directement des structures internes Fortran ou de la fenêtre principale. |

Les noms de dossiers ci-dessous sont proposés ; leur adoption dépendra du découpage effectif des sources :

```text
wsjtz/
  core/           # modèles et contrats internes Z
  adapters/       # raccordements au moteur, audio, radio et interface amont
  automation/     # Auto CQ, Auto Call, Pounce, files et changement de bande
  filtering/      # sélection, règles et mises en évidence
  integrations/   # extensions journal, UDP, QRZ, carte et PSK Reporter
  modes/          # capacités et comportements propres à Z, notamment FT2
  ui/             # panneaux et commandes Z
  decoding/       # extensions de décodage spécifiques à Z
```

Préférer des bibliothèques/cibles CMake distinctes pour les extensions et l'adaptation. L'application assemble les composants ; éviter les dépendances circulaires. Cette modularité ne nécessite pas des plugins chargés dynamiquement ni un déplacement massif des fichiers amont.

### Contrats et raccordements

- Exposer aux extensions Z un résultat de décodage structuré, avec mode, heure, fréquence, SNR, texte et informations de session. Traduire les sorties amont dans l'adaptateur ; ne pas propager partout leur format textuel ou leurs structures mémoire.
- Fournir des demandes d'exploitation explicites : sélection de station, message à transmettre, changement de fréquence et arrêt. Le contrôleur radio/audio amont conserve la gestion effective du CAT, du PTT et de la session d'émission.
- Décrire les capacités par mode : séquences synchronisées ou réception continue, messages disponibles, émission progressive et automatismes compatibles. JTTY doit pouvoir utiliser son fonctionnement continu sans hériter des hypothèses de cycle FT8.
- Séparer les réglages spécifiques Z et leur migration des réglages amont, tout en préservant les préférences existantes.
- Extraire progressivement les règles Z de `mainwindow.*` vers les modules ; la fenêtre affiche et transmet les événements. Réduire les modifications de son fichier `.ui` en utilisant, lorsque possible, des panneaux Z séparés.
- Garder les contrôles de compatibilité C/Fortran et de communication. Ne pas ajouter de champs Z arbitraires aux structures amont : utiliser une interface d'extension explicite, ou documenter et tester un contrat modifié des deux côtés si cela reste nécessaire.

### Cas des modifications au cœur du décodage

Les filtres d'affichage et automatismes peuvent être isolés après le décodage. Les améliorations algorithmiques FT8 ne peuvent pas nécessairement l'être : elles peuvent modifier les passes, seuils, données de travail ou stratégies du moteur lui-même.

Pour ces modifications, rechercher d'abord un point d'extension dans le moteur amont. À défaut, maintenir des correctifs limités et identifiés, avec justification et tests de décodage associés. Éviter une copie complète permanente du moteur, qui doublerait l'entretien. Toute modification amont indispensable doit être consignée dans un inventaire : fichier, fonction Z préservée, raison du raccordement et validation attendue.

### Procédure pour chaque mise à jour WSJT-X

1. Enregistrer le tag et le commit amont utilisés, ainsi que la version Z et les versions de dépendances.
2. Comparer la nouvelle référence à la précédente, puis importer les changements amont dans des commits distincts des adaptations Z.
3. Adapter les raccordements et réappliquer seulement les correctifs Z encore nécessaires ; supprimer les correctifs devenus équivalents à une fonction amont après vérification.
4. Exécuter les tests amont, les tests de contrat et les tests de non-régression Z, puis comparer les décodages sur un corpus WAV commun.
5. Valider JTTY, les modes conservés et les fonctions Z avant de produire le paquet bêta.

L'objectif est de contenir l'impact des mises à jour dans les adaptateurs et les correctifs recensés. Aucune architecture ne garantit l'absence de régression lorsqu'un protocole ou une interface amont change ; les tests de contrat et de non-régression font partie de cette séparation.

## Origine et maintenance indépendante de FT2

L'historique identifie explicitement **WSJT-X Improved** comme origine déclarée du portage du mode FT2 actif :

- `ec2a6037502bb177e93996d2224c97866b146a5b`, 13 avril 2026 : « Port FT2 mode from wsjtx-improved ». Ajout du chemin de réception dans `lib/decoder.f90` et du mode dans `widgets/mainwindow.{cpp,h,ui}`.
- `c467f6653c5f79d0122fcac411b3caf27f223e2e` : correction de la période à **3,75 s**, avec espacement des tons doublé par rapport à FT4. Ce commit corrige la description erronée du premier commit, qui présentait FT2 comme un FT4 ralenti.
- Les commits suivants complètent émission, modulation, affichage, journalisation, temporisation et corrections. Ils constituent des adaptations Z à préserver séparément.

Dans la branche examinée, la réception FT2 utilise `nmode=52` : étirement des échantillons par deux, appel à `my_ft4%decode`, puis conversion de DT et fréquence dans `ft2_decoded`. L'émission utilise 105 symboles à 288 échantillons/symbole dans `mainwindow.cpp`. Le dossier historique `lib/ft2` décrit au contraire un mode expérimental LDPC (128,90), 144 symboles, et ne représente pas ce chemin actif ; `CMakeLists.txt` n'en liste que `gfsk_pulse.f90`. Ne pas prendre ce dossier pour le module FT2 à mettre à jour.

WSJT-X Improved est maintenant nommé **WS — Digital Mode Suite**, par Uwe Risse DG2YCB et ses collaborateurs. Références à suivre :

- Site officiel : https://wsjt-x-improved.sourceforge.io/
- Versions et archives sources : https://sourceforge.net/projects/wsjt-x-improved/files/
- Archives de la série 3.1.0 : https://sourceforge.net/projects/wsjt-x-improved/files/WSJT-X_v3.1.0/Source%20code/
- Annonce des corrections FT2 de 3.1.0 260228 : https://sourceforge.net/p/wsjt-x-improved/mailman/message/59302224/

La lignée amont déclarée est identifiée, mais **la version exacte et l'archive source utilisées lors du portage Z ne sont pas consignées dans le commit d'introduction**. Une attribution fichier par fichier à une révision précise reste à établir par comparaison des archives historiques. Decodium/IU8LMC constitue une autre implémentation à distinguer ; il ne faut pas lui attribuer ce portage Z sur la seule présence du nom FT2.

Organisation requise pour les mises à jour FT2 :

1. Ajouter FT2 comme deuxième référence amont, indépendante du tag WSJT-X : fournisseur, URL, version datée, archive et SHA-256 (ou commit si disponible).
2. Conserver un inventaire des fichiers et portions repris, avec attribution, licences et liste des adaptations Z. Ne pas enregistrer une version d'origine supposée comme vérifiée.
3. Isoler les paramètres FT2, conversions temporelles/fréquentielles et orchestration dans le module de mode et son adaptateur au moteur FT4. Identifier explicitement la dépendance au FT4 de WSJT-X.
4. Comparer chaque nouvelle version WS/Improved à la référence FT2 précédemment intégrée ; sélectionner les changements utiles sans importer automatiquement toutes les fonctions de cette autre application.
5. Tester conjointement le FT2 mis à jour et le FT4 amont : émission/réception croisée, période 3,75 s, symboles, fréquence, DT, messages, faux décodages, journal et automatismes Z. Une évolution du moteur FT4 peut affecter FT2 même sans modification du module FT2.

Cette exigence définit le suivi des sources. Elle ne crée pas de surveillance automatique ni de mise à jour automatique du code.

## Suivi du WSJT-Z original et sélection des futures fonctions

La version personnelle doit suivre **trois lignées** : WSJT-X pour la base et JTTY, WS/Improved pour FT2, et le WSJT-Z original pour ses fonctions et corrections propres. Le nom de version personnel `3.3.0-beta1` n'efface pas la référence Z d'origine `2.0.19`.

### Référence Z vérifiée le 13 octobre 2026

- Dépôt amont Z : https://github.com/sq9fve/wsjt-z
- Tag : `v2.0.19`.
- Commit : `b3a06e89ae0174c51c8595d99e40ca179738eb0a`.
- Le tag distant officiel et le tag local pointent sur ce même commit. Au moment de la vérification, `master` officiel pointe également dessus ; le tag et le commit constituent la référence durable, contrairement à `master`.
- La branche personnelle `macos-build-maintenance` part de cette référence et aboutit à `e850c68` pour l'étude. Ses commits supplémentaires doivent être recensés séparément du code officiel Z.
- La comparaison `v2.0.19..e850c68` porte uniquement sur neuf fichiers de documentation, de procédure macOS et d'exclusion Git ; aucun fichier du moteur ou de l'interface applicative n'y est modifié. Le code applicatif de départ est donc celui du tag officiel Z 2.0.19.

Il est possible d'identifier exactement l'ensemble des sources de Z 2.0.19 par ce tag. L'attribution de chaque portion à un auteur ou à une lignée est plus limitée : le dépôt comporte un instantané initial avec des modifications déjà intégrées. Ni un commentaire `Z`, ni le dernier auteur affiché par `git blame` ne suffisent à prouver l'origine d'un algorithme. L'inventaire doit distinguer provenance vérifiée, provenance déclarée et provenance restant à déterminer.

### Organisation des évolutions

Créer lors de la modularisation une matrice de provenance et de conservation par fonction :

| Information à enregistrer | Utilité |
| --- | --- |
| Fonction et module personnel | Retrouver où porter une évolution après déplacement du code. |
| Fichiers et symboles dans Z 2.0.19 | Comparer avec les futures versions originales. |
| Lignée et référence source | Distinguer WSJT-X, WS/Improved, Z et les adaptations personnelles. |
| Modifications personnelles et justification | Préserver les choix de votre version. |
| Dépendances et tests | Évaluer la portée d'une correction ou d'une fonction nouvelle. |
| Décision d'import et référence intégrée | Savoir ce qui a été adopté, adapté, différé ou écarté. |

Conserver les références Git d'origine et importer les modifications dans des commits distincts et attribués. Éviter de réécrire ou d'aplatir l'historique servant de référence. Le déplacement des fonctions vers les modules personnels doit être effectué séparément des changements de comportement pour faciliter leur comparaison.

### À chaque nouvelle version officielle Z

1. Comparer son tag au dernier tag Z examiné, en commençant par `v2.0.19` ; examiner changelog, commits et différences de code.
2. Identifier corrections des fonctions conservées, nouvelles fonctionnalités, dépendances et modifications déjà reprises depuis WSJT-X ou WS/Improved. Éviter les imports en double.
3. Présenter les fonctions nouvelles avec leur intérêt, leur impact et leurs dépendances pour permettre votre sélection. Ne pas activer automatiquement toutes les nouveautés dans votre version.
4. Importer ou adapter les changements retenus dans les modules correspondants, en conservant les références des commits/PR Z sources. Un cherry-pick peut convenir pour un correctif isolé ; une adaptation explicite sera souvent nécessaire après modularisation.
5. Tester les fonctions concernées et les raccordements, puis mettre à jour la matrice. Enregistrer séparément la **dernière version examinée** et les **changements effectivement intégrés** : une sélection partielle ne signifie pas que toute une version Z a été adoptée.

La référence personnelle devra donc documenter simultanément la base WSJT-X, la référence FT2, le socle Z 2.0.19, les imports Z ultérieurs et les modifications personnelles. Cette procédure prépare le suivi futur ; aucune surveillance périodique n'est créée par cette étude.

## Ordre de réalisation recommandé

1. Établir une compilation témoin de la branche Z et figer les versions de dépendances macOS.
2. Inventorier et attribuer les différences Z/amont ; préparer une matrice de conservation par fonction, définir les contrats d'adaptation et extraire progressivement les extensions Z en modules dédiés.
3. Porter le moteur et ses contrats de communication, puis raccorder FT8 spécifique et FT2.
4. Intégrer JTTY de bout en bout, y compris réception, émission, interface et pilotage radio.
5. Appliquer la version `3.3.0-beta1` lorsque l'intégration est compilable ; harmoniser métadonnées et paquets.
6. Valider les tests puis produire un paquet bêta distinct pour essai.

## Critères de validation

- Compilation macOS avec les dépendances préservées, puis démarrage et reprise des réglages.
- Tests amont des interfaces C/Fortran, de communication et du moteur.
- Comparaison de décodage sur les mêmes fichiers WAV avec Z témoin et la nouvelle version : messages, doublons, faux décodages et durée. Ne pas promettre un gain de sensibilité avant mesure.
- Vérification des modes existants, en particulier FT8, FT4 et FT2.
- Tests JTTY amont de réception et de boucle émission/réception, puis essai audio contrôlé.
- Vérification des automatismes Z, filtres, journal et UDP.
- Essais CAT/PTT et arrêt d'émission ; les essais radio réels nécessitent le matériel et une intervention de l'opérateur.

## État de cette étude

Les deux sources exactes ont été téléchargées et inspectées. Aucun code de décodage ni numéro de version n'a encore été modifié. Aucun binaire 3.3.0-beta1 n'a été construit. Aucun changement n'a été envoyé au dépôt distant. Le test macOS déjà consigné dans la branche concerne Z 2.0.19 et valide seulement un démarrage et une reprise de réglages lors d'un essai bref.
