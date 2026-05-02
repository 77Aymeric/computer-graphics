## Trace TP — Illumination (diffuse + ambient) et normales

### PDF de référence
- `OpenGL - TP illumination - partie 01.pdf`

### Ce qu’on a fait dans ce rendu final
On n’a pas implémenté tout Phong/Blinn-Phong, mais on a repris le principe de base :
- utiliser les **normales** dans le fragment shader,
- calculer une composante **diffuse (Lambert)** + une petite **ambiante**,
afin de rendre la forme lisible et de vérifier que les normales sont bien transformées.

### Où le voir dans le code
- Fragment shader :
  - `MyPreparation/PreparationAuProjet/shaders/project.fs.glsl`
    - `ndotl = max(dot(N, L), 0)`
    - `diffuse + ambient`
- Normales :
  - `MyPreparation/PreparationAuProjet/shaders/project.vs.glsl` (`u_NormalMatrix * a_Normal`)
  - `MyPreparation/PreparationAuProjet/src/Math.hpp` (normal matrix)

