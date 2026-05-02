# Rapport TD2 OpenGL Moderne

## Partie A

### Exercice A.1

L'objectif de cet exercice etait d'ajouter une couleur a chaque sommet du triangle afin d'obtenir un degrade lors du rendu.

Le tableau de sommets a ete modifie pour contenir, pour chaque sommet :

- une position 2D : `x, y`
- une couleur RGB : `r, g, b`

Le shader vertex utilise maintenant deux attributs :

- `a_position`
- `a_color`

La couleur est transmise au fragment shader avec `v_color`, puis affichee avec `gl_FragColor`.

## Points importants

- Le triangle est defini dans un tableau interleave : position puis couleur pour chaque sommet.
- Le `stride` vaut `sizeof(float) * 5` car un sommet contient 5 flottants.
- L'attribut de position commence au debut du sommet.
- L'attribut de couleur commence apres les 2 flottants de position.

## Extrait de code

```cpp
static const float triangle[] = {
    -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
     0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
     0.0f,  0.5f, 0.0f, 0.0f, 1.0f
};

glEnableVertexAttribArray(loc_position);
glVertexAttribPointer(loc_position, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, triangle);

glEnableVertexAttribArray(loc_color);
glVertexAttribPointer(loc_color, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, triangle + 2);
```

## Resultat attendu

Le triangle doit s'afficher avec trois couleurs differentes aux sommets, ce qui produit un degrade sur toute la surface.

## Etat actuel

- `A1.cpp` contient l'initialisation, la terminaison et le rendu du triangle colore.
- `basic.vs` contient le vertex shader.
- `basic.fs` contient le fragment shader.

### Exercice A.2

L'objectif de cet exercice etait de remplacer le tableau de `float` par un tableau de `Vertex`.

Une structure `Vertex` a ete introduite pour regrouper les attributs d'un sommet :

- une position 2D
- une couleur RGB

Deux structures simples ont egalement ete utilisees :

- `Position2D` pour la position
- `ColorRGB` pour la couleur

Le triangle est maintenant defini sous forme de tableau de `Vertex`, ce qui rend les donnees plus lisibles et plus proches de la structure logique d'un sommet.

## Points importants pour A.2

- Le `stride` devient `sizeof(Vertex)`.
- L'attribut `a_position` commence a l'adresse de `triangle[0].position`.
- L'attribut `a_color` commence a l'adresse de `triangle[0].color`.
- Le principe du rendu ne change pas, seule l'organisation des donnees est modifiee.

## Extrait de code A.2

```cpp
struct Position2D
{
    float x;
    float y;
};

struct ColorRGB
{
    float r;
    float g;
    float b;
};

struct Vertex
{
    Position2D position;
    ColorRGB color;
};

static const Vertex triangle[] = {
    { { -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
    { {  0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f } },
    { {  0.0f,  0.5f }, { 0.0f, 0.0f, 1.0f } }
};
```

### Exercice B.1

L'objectif de cet exercice etait d'integrer un Vertex Buffer Object afin d'eviter de redefinir le triangle a chaque trame.

Le tableau de sommets n'est plus utilise directement dans la boucle de rendu. Il est maintenant transfere une seule fois au GPU dans la fonction `Initialise()` avec `glBufferData()`.

Un identifiant `VBO` a ete cree avec `glGenBuffers()`, puis lie avec `glBindBuffer(GL_ARRAY_BUFFER, VBO)`.

Dans la fonction `Render()`, les attributs ne pointent plus vers des adresses en memoire CPU, mais vers des offsets relatifs dans le buffer actuellement lie.

## Points importants pour B.1

- Le VBO est cree dans `Initialise()`.
- Les donnees du triangle sont envoyees au GPU avec `glBufferData()`.
- Le VBO est libere dans `Terminate()` avec `glDeleteBuffers()`.
- Le dernier parametre de `glVertexAttribPointer()` devient un offset relatif calcule avec `offsetof()`.

## Extrait de code B.1

```cpp
GLuint VBO;

glGenBuffers(1, &VBO);
glBindBuffer(GL_ARRAY_BUFFER, VBO);
glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle, GL_STATIC_DRAW);
glBindBuffer(GL_ARRAY_BUFFER, 0);

glBindBuffer(GL_ARRAY_BUFFER, VBO);
glVertexAttribPointer(loc_position, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, position));
glVertexAttribPointer(loc_color, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, color));
```

## Resultat attendu pour B.1

Le rendu obtenu reste le meme que precedemment, mais les donnees du triangle sont maintenant stockees dans un buffer OpenGL plutot que relues directement depuis un tableau CPU a chaque image.

### Exercice B.2

L'objectif de cet exercice etait de dessiner le triangle a l'aide d'un tableau d'indices avec `glDrawElements()` au lieu de `glDrawArrays()`.

Un tableau `indices` a ete ajoute pour decrire les sommets a utiliser dans le bon ordre. Ces indices sont ensuite transferes dans un Element Buffer Object, lie avec `GL_ELEMENT_ARRAY_BUFFER`.

La geometrie des sommets reste stockee dans le VBO, mais le dessin s'appuie maintenant sur les indices contenus dans l'EBO.

## Points importants pour B.2

- Un tableau d'indices a ete ajoute : `0, 1, 2`.
- Un `EBO` a ete cree avec `glGenBuffers()`.
- Les indices sont envoyes au GPU avec `glBufferData(GL_ELEMENT_ARRAY_BUFFER, ...)`.
- Le rendu se fait avec `glDrawElements(...)`.
- Le dernier parametre de `glDrawElements()` vaut `0`, car il represente ici un offset dans l'EBO lie.

## Extrait de code B.2

```cpp
GLuint EBO;

static const unsigned short indices[] = {
    0, 1, 2
};

glGenBuffers(1, &EBO);
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, 0);
```

## Resultat attendu pour B.2

Le rendu visuel reste identique a celui de `B.1`, mais le triangle est maintenant dessine en utilisant un tableau d'indices, ce qui prepare une organisation plus efficace pour des maillages contenant des sommets partages.

### Exercice B.3

L'objectif de cet exercice etait d'introduire un Vertex Array Object afin de memoriser le paramettrage des attributs et d'eviter de reconfigurer ces attributs dans la boucle de rendu.

Le `VAO` enregistre la correspondance entre le VBO, l'EBO et les attributs du shader. Cette configuration est realisee une seule fois dans `Initialise()`.

La fonction `Render()` est ainsi simplifiee : elle utilise le shader, lie le VAO, puis appelle directement `glDrawElements()`.

## Points importants pour B.3

- Un `VAO` a ete cree avec `glGenVertexArrays()`.
- Le paramettrage des attributs est deplace dans `Initialise()`.
- Le `VAO` reference le `VBO` et l'`EBO`.
- La boucle de rendu devient plus legere.
- Le `VAO` est libere dans `Terminate()` avec `glDeleteVertexArrays()`.

## Extrait de code B.3

```cpp
GLuint VAO;

glGenVertexArrays(1, &VAO);
glBindVertexArray(VAO);

glBindBuffer(GL_ARRAY_BUFFER, VBO);
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

glEnableVertexAttribArray(loc_position);
glVertexAttribPointer(loc_position, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, position));

glEnableVertexAttribArray(loc_color);
glVertexAttribPointer(loc_color, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, color));

glBindVertexArray(0);
```

## Resultat attendu pour B.3

Le rendu reste identique a `B.2`, mais la configuration des attributs n'est plus repetee a chaque image. Le code de `Render()` devient plus simple et plus proche de l'objectif vise par les VAO.
