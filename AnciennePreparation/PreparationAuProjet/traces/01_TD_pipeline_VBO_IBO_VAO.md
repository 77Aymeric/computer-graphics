## Trace TP/TD — Pipeline moderne (VBO / IBO / VAO)

### PDF de référence
- `TD OpenGL moderne 4A partie 01 (1).pdf`

### Ce qu’on a fait dans ce rendu final
- Création d’un **shader program** (vertex + fragment).
- Création d’un **VBO** (vertex buffer), d’un **EBO/IBO** (index buffer) et d’un **VAO**.
- Définition des **attributs** du vertex shader via `glVertexAttribPointer(...)`.

### Où le voir dans le code
- Base “squelette” : `correction_tp1_vbo_ibo_vao/correction_tp1.cpp` (mêmes étapes VBO/IBO/VAO).
- Réutilisé dans le rendu final :
  - `AnciennePreparation/PreparationAuProjet/src/DragonMesh.cpp` : `glGenBuffers`, `glBufferData`, VAO + attributs.
  - `AnciennePreparation/PreparationAuProjet/main.cpp` : chargement shaders + boucle de rendu.

