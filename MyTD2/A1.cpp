#define GL_SILENCE_DEPRECATION
#include "common/GLShader.h"
#include "GL/glew.h"
#include <GLUT/glut.h>

#ifdef WIN32
#include "GL/wglew.h"
#endif

GLShader g_BasicShader;
const int WINDOW_WIDTH = 960;
const int WINDOW_HEIGHT = 540;

bool Initialise()
{
    g_BasicShader.LoadVertexShader("basic.vs");
    g_BasicShader.LoadFragmentShader("basic.fs");
    g_BasicShader.Create();

    // cette fonction est spécifique à Windows et permet d’activer (1) ou non (0)
    // la synchronization vertical. Elle necessite l’include wglew.h
#ifdef WIN32
    wglSwapIntervalEXT(1);
#endif

    return true;
}

void Terminate()
{
    g_BasicShader.Destroy();
}

void Render()
{
    static const float triangle[] = {
        -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
         0.0f,  0.5f, 0.0f, 0.0f, 1.0f
    };

    // etape a. A vous de recuperer/passer les variables width/height
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    // etape b. Notez que glClearColor est un etat, donc persistant
    glClearColor(0.5f, 0.5f, 0.5f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    // etape c. on specifie le shader program a utiliser
    auto program = g_BasicShader.GetProgram();
    glUseProgram(program);

    // etape d.
    int loc_position = glGetAttribLocation(program, "a_position");
    int loc_color = glGetAttribLocation(program, "a_color");

    // premier parametre = 0, correspond ici au canal/emplacement du premier attribut
    // glEnableVertexAttribArray() indique que les donnees sont generiques (proviennent
    // d’un tableau) et non pas communes a tous les sommets
    glEnableVertexAttribArray(loc_position);
    glVertexAttribPointer(loc_position, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, triangle);

    glEnableVertexAttribArray(loc_color);
    glVertexAttribPointer(loc_color, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, triangle + 2);

    // etape e.
    // etape f. dessin de triangles dont la definition provient d’un tableau
    // le rendu s’effectue ici en prenant 3 sommets a partir du debut du tableau (0)
    glDrawArrays(GL_TRIANGLES, 0, 3);
    // on suppose que la phase d’echange des buffers front et back
    // le « swap buffers » est effectuee juste apres
}

void Display()
{
    Render();
    glutSwapBuffers();
    glutPostRedisplay();
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("Triangle");
    Initialise();
    glutDisplayFunc(Display);
    glutMainLoop();
    return 0;
}
