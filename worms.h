#pragma once
#include "util.h"

//------------------------------------------------------------------------------

const int   SCREEN_WIDTH      = 1200;
const int   SCREEN_HEIGHT     = 800;
const int   SCREEN_FPS        = 60;
const float POINT_SIZE        = 2;
const int   MAX_NUM_PARTICLES = 1024*32;

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
    int nextSegment;  // these can all be 16bit, assuming 64k max particles
    int prevSegment;
    int wormId;
    int nextInGrid;

    inline bool isTail() const { return prevSegment == -1; }  // can be both head and tail (all particles start that way)
    inline bool isHead() const { return nextSegment == -1; }
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
    uint32_t    m_tailFlags[MAX_NUM_PARTICLES / 32];
    static const int GRID_SIZE = 4;
    static const int GRID_COUNT = GRID_SIZE*GRID_SIZE;
    int         m_grid[GRID_COUNT];
    UpdateState m_state;
    float       m_stateTime;

public:
    WormsApp();

    void        update(float deltaTime);
    void        render();
    int         getTimerMs() const;

private:
    void        setState(UpdateState state) { m_state = state; m_stateTime = 0.0f; }
    void        reset(bool randPos, bool randVel);
    int         getNearestTail(Particle const& p, float * pDistSq = 0);
    bool        updateHeads(float deltaTime);  // returns true when on last worm
    void        updateTails(float deltaTime);
    void        updateExploding(float deltaTime);
    void        screenCollide(Particle& p);

    void        gridInsert(int idx);
    void        gridRemove(int idx, int cell);
    void        gridMove(int idx, const Vector2& oldPos);
    int         getGridCell(Vector2 const& pos);
};

//static_assert( sizeof(WormsApp) <= 1024*1024 );
