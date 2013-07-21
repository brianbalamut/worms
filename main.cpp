#include <stdio.h>
#include "freeglut.h"

static const int SCREEN_WIDTH  = 1280;
static const int SCREEN_HEIGHT = 960;
static const int SCREEN_FPS    = 60;

void gameloop(int val);
void update();
void render();


int main(int argc, char* args[])
{
    glutInit(&argc, args);
    glutInitContextVersion( 2, 1 );

    glutInitDisplayMode( GLUT_DOUBLE );
    glutInitWindowSize( SCREEN_WIDTH, SCREEN_HEIGHT );
    glutCreateWindow( "Worms" );


    //Initialize Projection Matrix
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();

    //Initialize Modelview Matrix
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    //Initialize clear color
    glClearColor( 0.f, 0.f, 0.f, 1.f );


    //Set rendering function
    glutDisplayFunc( render );

    //Set main loop
    glutTimerFunc( 1000 / SCREEN_FPS, gameloop, 0 );

    //Start GLUT main loop
    glutMainLoop();
}

void gameloop(int val)
{
    update();
    render();

    //Run frame one more time
    glutTimerFunc( 1000 / SCREEN_FPS, gameloop, val );
}

void update()
{
}

void render()
{
    glClear( GL_COLOR_BUFFER_BIT );

    glBegin( GL_QUADS );
        glVertex2f( -0.5f, -0.5f );
        glVertex2f(  0.5f, -0.5f );
        glVertex2f(  0.5f,  0.5f );
        glVertex2f( -0.5f,  0.5f );
    glEnd();

    glutSwapBuffers();
}
