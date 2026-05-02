#define GL_SILENCE_DEPRECATION
#include <math.h>
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#include <OpenGL/glu.h>
const int WINDOW_WIDTH = 960;
const int WINDOW_HEIGHT = 540;



void Render()
{
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (double)WINDOW_WIDTH / (double)WINDOW_HEIGHT, 0.1, 1000.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0.0, 3.0, 4.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    glBegin(GL_QUADS);
        glColor3f(0.3f, 0.3f, 0.3f);
        glVertex3f(-2.0f, -1.0f, -2.0f);
        glVertex3f( 2.0f, -1.0f, -2.0f);
        glVertex3f( 2.0f, -1.0f,  2.0f);
        glVertex3f(-2.0f, -1.0f,  2.0f);
    glEnd();

    glBegin(GL_TRIANGLES);
        glColor3f(1.f, 0.f, 0.f);
        glVertex3f(0.0f, 0.8f, 0.0f);

        glColor3f(0.f, 1.f, 0.f);
        glVertex3f(0.8f, -0.8f, 0.0f);

        glColor3f(0.f, 0.f, 1.f);
        glVertex3f(-0.8f, -0.8f, 0.0f);
    glEnd();
}


void Display()
{
    Render();
    glutSwapBuffers();
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE);
    glutInitWindowSize(960, 540);
    glutCreateWindow("Triangle");
    glutDisplayFunc(Display);
    glutMainLoop();
    return 0;
}
