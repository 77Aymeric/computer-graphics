## Trace TP — Rendu 3D simple (perspective, depth, culling) + Dragon

### PDF de référence
- `OpenGL - TP OpenGL moderne 02 - rendu 3D simple (1).pdf`

### Ce qu’on a fait dans ce rendu final
- Passage en **rendu 3D** avec une **projection perspective**.
- Activation du **Depth Test** et du **Back-face culling**.
- Affichage d’un modèle 3D indexé : ici le **Stanford Dragon**.

### Où le voir dans le code
- Rendu 3D + depth/culling :
  - `AnciennePreparation/PreparationAuProjet/main.cpp` :
    - `glEnable(GL_DEPTH_TEST)` + `glClear(GL_DEPTH_BUFFER_BIT)`
    - `glEnable(GL_CULL_FACE)` + `glCullFace(GL_BACK)`
    - `Perspective(...)`
- Dragon (VBO/EBO indexé) :
  - `AnciennePreparation/PreparationAuProjet/src/DragonMesh.cpp` :
    - `DragonVertices` + `DragonIndices` (dans `DragonData.h`)
    - `glDrawElements(...)`

