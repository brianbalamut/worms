#include "freeglut.h"
#include "worms.h"

void gameloop(int val);
void render();

static WormsApp s_app;

//------------------------------------------------------------------------------

int main(int argc, char* args[])
{
    // glut / window init
    glutInit(&argc, args);
    glutInitContextVersion(2, 1);
    glutInitDisplayMode(GLUT_DOUBLE);
    glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    glutCreateWindow("Worms");

    // opengl init (2d ortho cam)
    gluOrtho2D(0.0, SCREEN_WIDTH, 0.0, SCREEN_HEIGHT);
    glClearColor(0.0f, 0.0f, 0.1f, 1.0f);   
    glPointSize(POINT_SIZE);
    glColor3f(0.9f, 0.0f, 0.0f);

    // kick off glut loop
    glutDisplayFunc(render);
    glutTimerFunc(TIMER_MS, gameloop, 0);
    glutMainLoop();
    return 0;
}

void gameloop(int val)
{
    s_app.update(1.0f/SCREEN_FPS);
    render();

    glutTimerFunc(TIMER_MS, gameloop, val);

    // perf testing, run for X frames
    static int framesLeft = 30;
    if( --framesLeft <= 0 )
         glutLeaveMainLoop();
}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT);

    s_app.render();

    glutSwapBuffers();
}
