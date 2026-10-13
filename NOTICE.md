# Copyright, licences et crédits

Cette branche de travail prépare **WSJT-Fusion 1.0.0-beta1**, une version dérivée.
Elle n'est pas une publication officielle de WSJT-X, WS ou WSJT-Z.
Les modifications de cette branche ne transfèrent pas les droits des auteurs
du code repris. Les licences et mentions présentes dans chaque fichier restent
applicables ; ce document les complète sans les remplacer.

## Licence

WSJT-Z et le code WSJT repris sont distribués sous la GNU General Public
License, version 3 ou ultérieure, conformément aux notices des projets.
Le texte de la licence est conservé dans [COPYING](COPYING).
Des composants tiers comportent leurs propres notices : elles doivent être
conservées avec les fichiers correspondants et dans les distributions.

## Mention des auteurs WSJT

La notice suivante provient de `doc/common/license.adoc` de WSJT-X
`v3.3.0-beta1` et est également présente dans la source WS 3.2.1 examinée :

> The algorithms, source code, look-and-feel of WSJT-X and related
> programs, and protocol specifications for the modes FSK441, FST4,
> FST4W, FT4, FT8, JT4, JT6M, JT9, JT44, JT65, JTMS, Q65, QRA64, ISCAT,
> and MSK144 are Copyright (C) 2001-2026 by one or more of the following
> authors: Joseph Taylor, K1JT; Bill Somerville, G4WJS; Steven Franke,
> K9AN; Nico Palermo, IV3NWV; Uwe Risse, DG2YCB; Brian Moran, N9ADG;
> John Nelson, G4KLA; Charles Suckling, DL3WDG; Roger Rehr, W3SZ;
> Greg Beam, KI7MT; Michael Black, W9MDB; Edson Pereira, PY2SDR; Philip
> Karn, KA9Q; and other members of the WSJT Development Group.

Les attributions particulières dans les sources, `AUTHORS`, la documentation
et la fenêtre À propos doivent être conservées, y compris pour les modes
ou fichiers qui ne sont pas énumérés dans cette notice générale.

## Lignées et contributions

- **WSJT-X / WSJT Development Group** : base logicielle, protocoles,
  moteurs et outils. Référence prévue pour le moteur général et JTTY :
  [WSJT-X v3.3.0-beta1](https://github.com/WSJTX/wsjtx/tree/v3.3.0-beta1),
  commit `57ba9337fa020065dc0ceee2c4ef85e433ea32d4`.
  JTTY est développé par Joe Taylor K1JT et les contributeurs indiqués dans
  les sources et l'historique ; conserver leurs attributions spécifiques.
- **WSJT-Z / SQ9FVE et contributeurs** : extensions de fonctionnement,
  automatismes, filtres et intégrations. Socle :
  [WSJT-Z v2.0.19](https://github.com/sq9fve/wsjt-z/tree/v2.0.19),
  commit `b3a06e89ae0174c51c8595d99e40ca179738eb0a`.
- **WS, anciennement WSJT-X Improved / Uwe Risse DG2YCB et collaborateurs** :
  origine déclarée du portage FT2 actif dans l'historique Z.
  Référence source examinée pour sa mise à jour : WS 3.2.1, build `261010`,
  [archive officielle](https://sourceforge.net/projects/wsjt-x-improved/files/WS_v3.2.1/Source%20code/ws-3.2.1_261010.tgz/download).
  La version précise utilisée pour le portage initial Z reste à déterminer.
- **JTDX et ses contributeurs** : améliorations de décodage FT8 reprises
  dans le socle Z. L'instantané initial Z déclare cette reprise ; l'attribution
  détaillée à une révision JTDX reste à établir. Les en-têtes existants et
  les crédits JTDX doivent être conservés.
- **Branche personnelle Edpublic06** : maintenance macOS, documentation
  et adaptations d'intégration identifiées dans l'historique de cette branche.
  Ces adaptations ne doivent pas être attribuées aux projets amont.

L'identification de WS/Improved comme source FT2 du portage Z ne constitue
pas une attribution à Decodium/IU8LMC. Toute reprise future d'une autre
implémentation doit faire l'objet d'une provenance et de crédits distincts.

## État de l'intégration

Les références de mise à jour sont des sources examinées et des cibles
d'intégration, pas une déclaration que tous leurs changements sont déjà
intégrés ou validés. Le registre [UPSTREAM-SOURCES.md](UPSTREAM-SOURCES.md)
doit suivre les imports effectifs et leurs adaptations.

Pour toute distribution : conserver les notices, signaler les fichiers
modifiés et leurs dates, inclure les licences applicables et fournir le code
source correspondant conformément à celles-ci. Les binaires et les sources
publiés doivent correspondre à la même révision.

## Identité du fork

WSJT-Fusion est un fork indépendant maintenu par F6FDY, dérivé de WSJT-Z 2.0.19 de SQ9FVE. Son numéro de version est indépendant des versions des composants. Les copyrights et crédits historiques restent applicables ; F6FDY ne revendique pas la paternité du code repris.
