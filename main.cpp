#include "freeglut.h"

static const int   SCREEN_WIDTH      = 800;
static const int   SCREEN_HEIGHT     = 600;
static const int   SCREEN_FPS        = 60;
static const float POINT_SIZE        = 3;
static const int   MAX_NUM_PARTICLES = 1024;

void gameloop(int val);
void update();
void render();

//------------------------------------------------------------------------------

struct Particle
{
    float pos_x, pos_y;
    float vel_x, vel_y;
};

class WormsApp
{
    Particle particles[MAX_NUM_PARTICLES];

public:
    WormsApp()
    {
        for( int i=0; i < MAX_NUM_PARTICLES; ++i )
        {
            particles[i].pos_x = rand() % SCREEN_WIDTH;
            particles[i].pos_y = rand() % SCREEN_HEIGHT;
            particles[i].vel_x = (rand() % 100) - 50;
            particles[i].vel_y = (rand() % 100) - 50;
        }
    }

    void update(float deltaTime)
    {
        for( int i=0; i < MAX_NUM_PARTICLES; ++i )
        {
            Particle& p = particles[i];
            p.pos_x += p.vel_x * deltaTime;
            p.pos_y += p.vel_y * deltaTime;
            p.vel_x *= 0.995f;
            p.vel_y *= 0.995f;
        }
    }

    void render()
    {
        glBegin(GL_POINTS);
        for( int i=0; i < MAX_NUM_PARTICLES; ++i )
        {
            glVertex2f(particles[i].pos_x, particles[i].pos_y);
        }
        glEnd();
    }
};

static WormsApp s_app;

//------------------------------------------------------------------------------

int main(int argc, char* args[])
{
    // glut / window init
    glutInit(&argc, args);
    glutInitContextVersion( 2, 1 );
    glutInitDisplayMode( GLUT_DOUBLE );
    glutInitWindowSize( SCREEN_WIDTH, SCREEN_HEIGHT );
    glutCreateWindow( "Worms" );

    // opengl init (2d ortho cam)
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    gluOrtho2D(0.0, SCREEN_WIDTH, 0.0, SCREEN_HEIGHT);
    glClearColor(0.0f, 0.1f, 0.0f, 1.0f);   
    glPointSize(POINT_SIZE);

    // kick off glut loop
    glutDisplayFunc(render);
    glutTimerFunc(1000 / SCREEN_FPS, gameloop, 0);
    glutMainLoop();
}

void gameloop(int val)
{
    update();
    render();

    glutTimerFunc(1000 / SCREEN_FPS, gameloop, val);
}

void update()
{
    s_app.update(1.0f/SCREEN_FPS);
}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT);

    s_app.render();

    glutSwapBuffers();
}
