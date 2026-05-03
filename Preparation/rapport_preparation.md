# Préparation au projet OpenGL

## Introduction

Ce document présente la préparation réalisée dans le dossier `Preparation/`. Il suit la logique de la consigne sous forme `exercice -> réponse -> explication`, puis résume l'intégration finale avec les notions reprises des anciens TD/TP.

Le programme affiche le Stanford Dragon avec une caméra orbitale, une matrice de vue calculée manuellement, une chaîne `Model/View/Projection`, et un éclairage simple basé sur les normales.

![Vue générale du rendu](ScreenShots/Screenshot%202026-05-03%20at%2015.36.12.png)

Cette première capture montre le rendu principal : le dragon est affiché avec une projection perspective, un fond sombre et un éclairage qui permet de lire le volume.

Fichiers principaux :

- `preparation.cpp` : code C++ principal.
- `shaders/project.vs.glsl` : vertex shader.
- `shaders/project.fs.glsl` : fragment shader.
- `common/GLShader.*` : utilitaire de compilation des shaders.
- `Makefile` : compilation et lancement.

## Exercice 1.1 - Multiplication de matrices

### Réponse

Une fonction `Multiply` a été ajoutée pour multiplier deux matrices homogènes 4x4 :

```cpp
Mat4 Multiply(const Mat4& A, const Mat4& B)
```

Les matrices sont stockées en colonne-major, comme attendu par OpenGL. L'accès à un coefficient se fait donc avec :

```cpp
matrix[col * 4 + row]
```

La formule utilisée est :

\[
C_{row,col} = \sum_k A_{row,k} \cdot B_{k,col}
\]

### Explication

Cette fonction sert à préparer la composition de transformations. Au lieu d'envoyer séparément une matrice de translation, une matrice de rotation et une matrice de scale, on peut construire une seule matrice monde.

Le principe est :

\[
World = Translation \cdot Rotation \cdot Scale
\]

Avec des vecteurs colonnes, l'ordre d'application se lit de droite à gauche. Un sommet local est donc d'abord mis à l'échelle, puis tourné, puis déplacé dans la scène.

## Exercice 1.2 - Remplacer les transformations locales par une World Matrix

### Réponse

Le programme utilise une matrice unique `u_Model` pour représenter la transformation locale vers monde :

```cpp
Mat4 model = Identity();
```

Cette matrice est envoyée au vertex shader :

```cpp
glUniformMatrix4fv(locModel, 1, GL_FALSE, model.m.data());
```

Dans le shader, la position du sommet est transformée avec :

```glsl
vec4 worldPos = u_Model * vec4(a_Position, 1.0);
```

### Explication

Dans cette préparation, `u_Model` vaut l'identité car le dragon est déjà affiché dans une position correcte. Le point important est que le pipeline est prêt pour une vraie transformation monde unique.

La position suit ensuite la chaîne :

\[
clipPosition = Projection \cdot View \cdot Model \cdot position
\]

Ce choix simplifie le shader : il reçoit une matrice `Model`, une matrice `View` et une matrice `Projection`, au lieu de recevoir plusieurs transformations locales séparées.

## Exercice 1.3 - Normal Matrix

### Réponse

Les normales ne sont pas transformées directement par `u_Model`. Une matrice dédiée est calculée côté CPU :

```cpp
static Mat3 NormalMatrixFromModel(const Mat4& model)
{
    Mat3 m3 = Mat3FromMat4(model);
    return Transpose(Inverse(m3));
}
```

Elle correspond à :

\[
NormalMatrix = transpose(inverse(mat3(Model)))
\]

Elle est ensuite envoyée au shader avec :

```cpp
glUniformMatrix3fv(locNormalMat, 1, GL_FALSE, normalMat.m.data());
```

Dans le vertex shader :

```glsl
v_Normal = normalize(u_NormalMatrix * a_Normal);
```

### Explication

Une normale est une direction, pas une position. Elle ne doit donc pas être affectée par une translation. De plus, si une transformation contient un scale non uniforme, appliquer directement la matrice `Model` sur la normale peut donner un résultat faux.

La normal matrix corrige ce problème. Même si, dans ce rendu, `Model` vaut l'identité, le code respecte la formule attendue et reste correct si une transformation est ajoutée plus tard.

## Exercice 2.1 - Fonction LookAt

### Réponse

La fonction `LookAt` a été codée manuellement :

```cpp
Mat4 LookAt(const Vec3& position, const Vec3& target, const Vec3& up)
```

Elle calcule d'abord les axes de la caméra :

\[
forward = normalize(position - target)
\]

\[
right = normalize(up \times forward)
\]

\[
up2 = forward \times right
\]

Puis elle calcule la translation inverse :

\[
tx = -dot(position, right)
\]

\[
ty = -dot(position, up2)
\]

\[
tz = -dot(position, forward)
\]

La matrice finale est renvoyée et utilisée comme view matrix.

### Explication

La view matrix transforme les coordonnées du monde vers le repère de la caméra. Dans le repère caméra, la caméra est toujours à l'origine.

La fonction `LookAt` reconstruit donc une base orthonormale à partir de la position de caméra, de la cible et du vecteur `up`. Ensuite, les produits scalaires permettent de projeter la position de la caméra sur cette base pour obtenir la translation inverse.

Cette matrice est envoyée au shader via `u_View`.

## Exercice 2.2 - Utilisation de la View Matrix dans le shader

### Réponse

La view matrix est appliquée dans le vertex shader avant la projection :

```glsl
gl_Position = u_Projection * u_View * worldPos;
```

Les normales ne reçoivent pas `u_View`. Elles sont uniquement transformées par `u_NormalMatrix`.

### Explication

La position d'un sommet doit passer successivement :

1. du repère local vers le monde avec `u_Model`;
2. du monde vers la caméra avec `u_View`;
3. de la caméra vers le clip space avec `u_Projection`.

Les normales restent dans l'espace utilisé pour l'éclairage. Ici, l'éclairage est cohérent avec les normales transformées par la matrice issue de `Model`.

## Exercice 3 - Caméra orbitale

### Réponse

Une structure `OrbitCamera` a été ajoutée. Elle contient :

- `target` : le point regardé;
- `distance` : la distance entre la caméra et la cible;
- `phi` : l'azimut;
- `theta` : l'élévation.

La position de la caméra est calculée en coordonnées sphériques :

\[
Y = R \cdot sin(theta)
\]

\[
X = R \cdot cos(theta) \cdot cos(phi)
\]

\[
Z = R \cdot cos(theta) \cdot sin(phi)
\]

Puis la vue est obtenue avec :

```cpp
return LookAt(eye, target, up);
```

### Explication

La caméra se déplace sur une sphère autour de la cible. Le rayon de cette sphère est `distance`.

Les entrées GLFW contrôlent les paramètres :

- clic gauche maintenu + déplacement horizontal : modification de `phi`;
- clic gauche maintenu + déplacement vertical : modification de `theta`;
- molette : modification de `distance`.

Des limites sont appliquées :

- `phi` est ramené dans `[-PI, PI]`;
- `theta` est limité pour éviter que la caméra se retourne;
- `distance` est limitée entre `minDistance` et `maxDistance`.

![Caméra orbitale - autre angle](ScreenShots/Screenshot%202026-05-03%20at%2015.36.14.png)

Cette capture illustre l'intérêt de la caméra orbitale : le dragon reste la cible de la caméra, mais le point de vue change en modifiant les angles `phi` et `theta`.

## Exercice 4 - Affichage du dragon

### Réponse

Le dragon est chargé depuis `Context/DragonData.h`. Le tableau de sommets contient 8 flottants par sommet :

- 3 flottants pour la position;
- 3 flottants pour la normale;
- 2 flottants pour les UV.

Dans ce rendu, seules les positions et les normales sont utilisées. Les données sont envoyées au GPU dans :

- un VBO pour les sommets;
- un IBO pour les indices;
- un VAO pour mémoriser la configuration.

Le rendu utilise :

```cpp
glDrawElements(GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_SHORT, nullptr);
```

### Explication

Cette partie reprend directement les anciens TD sur le pipeline OpenGL moderne. Le VBO évite de renvoyer les sommets à chaque frame. L'IBO permet de réutiliser les sommets grâce à un tableau d'indices. Le VAO évite de reconfigurer les attributs à chaque rendu.

Les attributs du shader sont :

- `a_Position` pour la position;
- `a_Normal` pour la normale.

Les UV sont gardées dans le stride, mais elles ne sont pas utilisées car le texturing n'est pas nécessaire pour cette préparation.

![Dragon vu de dos](ScreenShots/Screenshot%202026-05-03%20at%2015.36.20.png)

Cette vue confirme que le maillage est rendu en 3D avec les indices, la profondeur et le culling. Le changement d'angle permet aussi de vérifier que la matrice de vue est bien recalculée par la caméra orbitale.

## Exercice 5 - Projection perspective, depth test et culling

### Réponse

Le programme active le depth test et le back-face culling :

```cpp
glEnable(GL_DEPTH_TEST);
glDepthFunc(GL_LEQUAL);
glEnable(GL_CULL_FACE);
glCullFace(GL_BACK);
```

La projection perspective est calculée avec :

```cpp
Perspective(60.0f * pi / 180.0f, aspect, 0.1f, 100.0f);
```

### Explication

Le depth test permet d'afficher correctement les faces selon leur profondeur. Sans lui, des triangles éloignés pourraient apparaître devant des triangles proches.

Le culling supprime les faces arrière. Cela améliore le rendu et évite de dessiner des faces qui ne devraient pas être visibles.

La projection perspective donne une impression de profondeur : les objets éloignés apparaissent plus petits que les objets proches.

## Exercice 6 - Illumination ambiante et diffuse

### Réponse

Le fragment shader calcule une lumière diffuse de Lambert :

```glsl
float LambertDiffuse = max(dot(N, L), 0.0);
```

Il ajoute aussi une ambiance hémisphérique :

```glsl
vec3 hemiAmbient = mix(groundColor, skyColor, t);
```

Enfin, une correction gamma est appliquée :

```glsl
vec3 colorSRGB = pow(colorLinear, vec3(1.0 / 2.2));
```

### Explication

La lumière diffuse dépend de l'angle entre la normale et la direction de lumière. Plus la normale pointe vers la lumière, plus la surface est éclairée.

L'ambiance hémisphérique donne une couleur différente selon l'orientation de la normale : une surface orientée vers le haut reçoit davantage la couleur du ciel, tandis qu'une surface orientée vers le bas reçoit davantage la couleur du sol.

La correction gamma rend le résultat visuel plus naturel sur un écran.

### Captures des étapes d'éclairage

Les captures ajoutées après le pull montrent les étapes successives utilisées pour arriver au rendu final.

![Étape 1 - Diffuse seule](ScreenShots/00_diffuse_only.png)

Avec seulement la lumière diffuse, les zones qui ne sont pas orientées vers la lumière deviennent très sombres. Cela permet de vérifier que le produit scalaire entre la normale et la direction de lumière fonctionne, mais le modèle reste difficile à lire dans les ombres.

![Étape 2 - Ambiante constante](ScreenShots/01_ambient_constant.png)

L'ajout d'une ambiante constante rend les zones sombres visibles. Le volume est plus lisible, mais l'éclairage reste uniforme dans les parties non directement éclairées.

![Étape 3 - Ambiante hémisphérique et gamma](ScreenShots/02_hemi_gamma.png)

La version finale utilise une ambiance hémisphérique et une correction gamma. Les surfaces orientées vers le haut reçoivent une teinte plus claire/froide, les surfaces orientées vers le bas restent légèrement plus sombres, et l'image paraît moins dure.

Les captures précédentes montrent donc le rôle de chaque ajout : diffuse pour le relief, ambiante pour la lisibilité, hémisphérique et gamma pour un rendu plus agréable.

## Intégration finale

À chaque frame, la boucle de rendu effectue les étapes suivantes :

1. récupérer la taille du framebuffer;
2. mettre à jour le viewport;
3. effacer la couleur et la profondeur;
4. recalculer la projection avec le bon ratio;
5. calculer la view matrix avec la caméra orbitale;
6. calculer la normal matrix;
7. envoyer les uniforms au shader;
8. dessiner le dragon.

Cette structure regroupe les notions vues dans les anciens TD/TP : pipeline moderne, rendu indexé, projection 3D, transformations, caméra, normales et illumination.

## Compilation et exécution

Depuis `Preparation/` :

```bash
make
make run
```

Le binaire généré est :

```bash
bin/preparation
```

Le Makefile compile en C++17 et lie GLFW, GLEW ainsi que les frameworks OpenGL nécessaires sur macOS.

## Validation

Points validés dans cette préparation :

- [x] Affichage d'un objet 3D.
- [x] Utilisation du Stanford Dragon.
- [x] VBO pour les sommets.
- [x] IBO pour les indices.
- [x] VAO pour la configuration des attributs.
- [x] Projection perspective.
- [x] Depth test.
- [x] Back-face culling.
- [x] Fonction de multiplication de matrices 4x4.
- [x] Matrice `Model` unique.
- [x] Fonction `LookAt` codée manuellement.
- [x] View matrix envoyée au vertex shader.
- [x] Caméra orbitale contrôlée avec la souris et la molette.
- [x] Normal matrix calculée depuis `Model`.
- [x] Éclairage diffus.
- [x] Ambiance hémisphérique.
- [x] Correction gamma.

## Améliorations possibles

Le rendu actuel est volontairement centré sur la préparation demandée. Pour aller plus loin, on pourrait :

- ajouter une vraie transformation `Model` avec translation, rotation et scale;
- utiliser les UV du dragon pour ajouter une texture;
- ajouter un contrôle clavier pour déplacer ou modifier la lumière;
- sauvegarder des captures d'écran pour illustrer plusieurs vues orbitales;
- ajouter un mode fil de fer pour mieux visualiser la géométrie.
