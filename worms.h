#pragma once
#include "util.h"

//------------------------------------------------------------------------------

const int   SCREEN_WIDTH      = 1200;
const int   SCREEN_HEIGHT     = 800;
const int   SCREEN_FPS        = 60;
const int   TIMER_MS          = 16;
const float POINT_SIZE        = 1;
const int   MAX_NUM_PARTICLES = 1024*16;

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
    int         m_tails[MAX_NUM_PARTICLES];
    int         m_numTails;
    UpdateState m_state;
    float       m_stateTime;

public:
    WormsApp();

    void        update(float deltaTime);
    void        render();

private:
    void        setState(UpdateState state) { m_state = state; m_stateTime = 0.0f; }
    void        reset(bool randPos, bool randVel);
    bool        updateHeads(float deltaTime);  // returns true when on last worm
    void        updateTails(float deltaTime);
    void        updateExploding(float deltaTime);
    int         getNearestTail(Particle const& p, float * pDistSq = 0);
};

//static_assert( sizeof(WormsApp) <= 1024*1024 );
