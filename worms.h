#pragma once
#include "util.h"

//------------------------------------------------------------------------------

const int      SCREEN_WIDTH      = 1200;
const int      SCREEN_HEIGHT     = 800;
const int      SCREEN_FPS        = 60;
const float    POINT_SIZE        = 2;
const int      MAX_NUM_PARTICLES = (1024*42);
const uint16_t INVALID_INDEX     = 0xFFFF;

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
    uint16_t nextSegment;
    uint16_t prevSegment;
    uint16_t wormId;
    uint16_t nextInGrid;

    inline bool isTail() const { return prevSegment == INVALID_INDEX; }  // can be both head and tail (all particles start that way)
    inline bool isHead() const { return nextSegment == INVALID_INDEX; }
};

STATIC_ASSERT( sizeof(Particle) == 24 );

//------------------------------------------------------------------------------

class WormsApp
{
    enum UpdateState
    {
        kUS_Normal,
        kUS_LastWorm,
        kUS_Exploding,
    };

    static const int GRID_SIZE = 4;
    static const int GRID_COUNT = GRID_SIZE*GRID_SIZE;
    Particle    m_particles[MAX_NUM_PARTICLES];
    uint16_t    m_grid[GRID_COUNT];
    float       m_stateTime;
    UpdateState m_state;

public:
    WormsApp();

    void        update(float deltaTime);
    void        render();
    int         getTimerMs() const;

private:
    void        setState(UpdateState state) { m_state = state; m_stateTime = 0.0f; }
    void        reset(bool randPos, bool randVel);
    uint16_t    getNearestTail(Particle const& p, float * pDistSq = 0);
    bool        updateHeads(float deltaTime);  // returns true when on last worm
    void        updateTails(float deltaTime);
    void        updateExploding(float deltaTime);
    void        screenCollide(Particle& p);

    void        gridInsert(uint16_t idx);
    void        gridRemove(uint16_t idx, int cell);
    void        gridMove(uint16_t idx, const Vector2& oldPos);
    int         getGridCell(Vector2 const& pos);
};

STATIC_ASSERT( sizeof(WormsApp) <= 1024*1024 );
