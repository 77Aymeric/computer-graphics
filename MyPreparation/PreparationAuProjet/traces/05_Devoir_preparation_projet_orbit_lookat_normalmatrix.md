## Trace — Devoir “Préparation au projet” (orbit, LookAt, view matrix, normal matrix)

### Énoncé de référence
- Devoir Blackboard : “TD OpenGL - préparation au projet”
- PDF attaché : `OpenGL - TP A RENDRE - Préparation au Projet.pdf` (non présent localement ici, mais énoncé copié dans le chat)

### Exigences obligatoires et preuve dans le code
1) **Caméra orbitale**
- `MyPreparation/PreparationAuProjet/src/OrbitCamera.cpp`
  - `yaw/pitch/distance`
  - `OnMouseDrag`, `OnScroll`

2) **LookAt qui génère une view matrix**
- `MyPreparation/PreparationAuProjet/src/Math.hpp`
  - `LookAt(eye, target, up)` retourne `Mat4`

3) **Appliquer la view matrix au vertex shader sur la position uniquement**
- `MyPreparation/PreparationAuProjet/shaders/project.vs.glsl`
  - `gl_Position = u_Projection * u_View * (u_Model * vec4(a_Position,1))`
  - normales : pas de `u_View`

4) **Normal matrix dédiée (ou world)**
- `MyPreparation/PreparationAuProjet/src/Math.hpp`
  - `NormalMatrixFromModel(model)` = `transpose(inverse(mat3(model)))`
- `MyPreparation/PreparationAuProjet/main.cpp` :
  - envoi `u_NormalMatrix` séparée

