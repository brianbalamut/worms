#pragma once

//------------------------------------------------------------------------------

const int   SCREEN_WIDTH      = 600;
const int   SCREEN_HEIGHT     = 600;
const int   SCREEN_FPS        = 60;
const float POINT_SIZE        = 2;
const int   MAX_NUM_PARTICLES = 256;

//------------------------------------------------------------------------------

enum ParticleState
{
    kPS_Head,
    kPS_Tail,
    kPS_Segment,
};

struct Particle
{
    float pos_x, pos_y;
    float vel_x, vel_y;
    int nextSegment;
    int prevSegment;
    int wormId;
};

class WormsApp
{
    Particle particles[MAX_NUM_PARTICLES];

public:
    WormsApp();

    void update(float deltaTime);
    void render();
};

//static_assert( sizeof(WormsApp) <= 1024*1024 );
