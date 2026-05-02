#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
const int WINDOW_WIDTH = 960;
const int WINDOW_HEIGHT = 540;
void Render()
{
    // Viewport 1 : bas gauche
    glViewport(0, 0, WINDOW_WIDTH/2, WINDOW_HEIGHT/2);
    glScissor(0, 0, WINDOW_WIDTH/2, WINDOW_HEIGHT/2);
    glClearColor(1.f, 1.f, 0.f, 1.f);
    glEnable(GL_SCISSOR_TEST);
    glClear(GL_COLOR_BUFFER_BIT);

    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    float t = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    glRotatef(t * 50.0f, 0.f, 0.f, 1.f);
    glBegin(GL_TRIANGLES);
        glColor3f(1.f, 0.f, 0.f);
        glVertex2f(0.0f, 0.6f);

        glColor3f(0.f, 1.f, 0.f);
        glVertex2f(0.6f, -0.6f);

        glColor3f(0.f, 0.f, 1.f);
        glVertex2f(-0.6f, -0.6f);
    glEnd();
    
    // Viewport 2 : bas droite
    glViewport(WINDOW_WIDTH / 2, 0, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
    glScissor(WINDOW_WIDTH / 2, 0, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
    glClearColor(0.f, 1.f, 1.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    glBegin(GL_LINES);
        glColor3f(1.f, 0.f, 0.f);
        glVertex2f(-0.8f, -0.8f);

        glColor3f(0.f, 0.f, 1.f);
        glVertex2f(0.8f, 0.8f);
    glEnd();


    // Viewport 3 : haut gauche
    glViewport(0, WINDOW_HEIGHT / 2, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
    glScissor(0, WINDOW_HEIGHT / 2, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
    glClearColor(1.f, 0.f, 1.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    glBegin(GL_TRIANGLE_STRIP);
        glColor3f(1.f, 0.f, 0.f);
        glVertex2f(-0.7f, -0.6f);

        glColor3f(0.f, 1.f, 0.f);
        glVertex2f(-0.3f, 0.6f);

        glColor3f(0.f, 0.f, 1.f);
        glVertex2f(0.2f, -0.6f);

        glColor3f(1.f, 1.f, 0.f);
        glVertex2f(0.7f, 0.6f);
    glEnd();

    // Viewport 4 : haut droite
    glViewport(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
    glScissor(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
    glClearColor(0.5f, 0.5f, 0.5f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    glBegin(GL_QUADS);
        glColor3f(1.f, 0.f, 0.f);
        glVertex2f(-0.5f, -0.5f);

        glColor3f(0.f, 1.f, 0.f);
        glVertex2f(0.5f, -0.5f);

        glColor3f(0.f, 0.f, 1.f);
        glVertex2f(0.5f, 0.5f);

        glColor3f(1.f, 1.f, 0.f);
        glVertex2f(-0.5f, 0.5f);
    glEnd();

    glDisable(GL_SCISSOR_TEST);
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
    glutInitWindowSize(960, 540);
    glutCreateWindow("Triangle");
    glutDisplayFunc(Display);
    glutMainLoop();
    return 0;
}
