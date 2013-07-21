#pragma once
#include "util.h"

//------------------------------------------------------------------------------

const int   SCREEN_WIDTH      = 800;
const int   SCREEN_HEIGHT     = 600;
const int   SCREEN_FPS        = 60;
const float POINT_SIZE        = 2;
const int   MAX_NUM_PARTICLES = 1024;

//------------------------------------------------------------------------------

enum ParticleState
{
    kPS_Head,
    kPS_Tail,
    kPS_Segment,
};

struct Particle
{
    Vector2 pos;
    Vector2 vel;
    int nextSegment;
    int prevSegment;
    int wormId;
};


class WormsApp
{
    enum UpdateState
    {
        kUS_Normal,
        kUS_LastWorm,
        kUS_Exploding,
    };

    Particle    m_particles[MAX_NUM_PARTICLES];
    UpdateState m_state;

public:
    WormsApp();

    void        update(float deltaTime);
    void        render();

private:
    void        reset(bool randPos, bool randVel);
    UpdateState updateHeads(float deltaTime);
    void        updateTails(float deltaTime);
};

//static_assert( sizeof(WormsApp) <= 1024*1024 );
