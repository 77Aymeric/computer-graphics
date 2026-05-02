## Soumission Blackboard — TD OpenGL : préparation au projet (binôme)

### Contenu à rendre
- **Rapport** : `rapport_projet.pdf` (ou `rapport_projet.md` si le prof accepte Markdown)
- **Code** : `PreparationAuProjet.zip` (sources + shaders)

### Comment lancer
Le code est basé sur **GLFW + GLEW** (même squelette que `correction_tp1_vbo_ibo_vao/`).

Dans ce dossier, les fichiers importants sont :
- `main.cpp` : point d’entrée (création fenêtre, rendu, input)
- `src/Math.hpp` : `LookAt`, `Perspective`, normal matrix
- `src/OrbitCamera.*` : caméra orbitale
- `src/DragonMesh.*` : VAO/VBO/EBO du dragon depuis `DragonData.h`
- `shaders/project.vs.glsl`, `shaders/project.fs.glsl`
- `traces/` : preuves “TP → où ça se voit dans le code”

### Screenshots
Les captures sont dans `screens/` :
- `01_overview.png`
- `02_orbit_left.png`
- `03_orbit_right.png`
- `04_zoom.png`

### Touches (prévu)
- **Clique + glisser** : orbite (yaw/pitch)
- **Molette** : zoom
- **P** : capture d’écran (écrit un fichier dans `screens/`)

