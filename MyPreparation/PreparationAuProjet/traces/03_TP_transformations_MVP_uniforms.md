## Trace TP — Transformations (M, V, P) + uniforms matrices

### PDF de référence
- `OpenGL - TP Transformations.pdf`

### Ce qu’on a fait dans ce rendu final
- Calcul côté CPU de :
  - `M` (model/world) : rotation Y
  - `V` (view) : caméra orbitale via LookAt
  - `P` (projection) : perspective
- Envoi au shader avec `glUniformMatrix4fv` / `glUniformMatrix3fv`.

### Où le voir dans le code
- Matrices + uniforms :
  - `MyPreparation/PreparationAuProjet/main.cpp` :
    - `Mat4 projection = Perspective(...)`
    - `Mat4 view = g_Camera.GetViewMatrix()`
    - `Mat4 model = RotationY(...)`
    - `glUniformMatrix4fv(u_Model/u_View/u_Projection, ...)`
- Calcul `LookAt` / `Perspective` :
  - `MyPreparation/PreparationAuProjet/src/Math.hpp`
- Vertex shader (application MVP) :
  - `MyPreparation/PreparationAuProjet/shaders/project.vs.glsl`

