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

    static const int GRID_SIZE = 10;
    static const int GRID_COUNT = GRID_SIZE*GRID_SIZE;
    Particle    m_particles[MAX_NUM_PARTICLES];
    uint16_t    m_grid[GRID_COUNT];
    uint8_t     m_gridSearchOrder[GRID_COUNT][GRID_COUNT]; // flatten the search recursion
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
    void        getNearestInCell(Particle const& p, int cell, uint16_t& nearestIdx, float& nearestDistSq);
    uint16_t    getNearestTail(Particle const& p, float * pDistSq = 0);
    bool        updateHeads(float deltaTime);  // returns true when on last worm
    void        updateTails(float deltaTime);
    void        updateExploding(float deltaTime);
    void        screenCollide(Particle& p);


    struct GridCell2d
    {
        uint16_t x, y;

        GridCell2d(uint16_t _x, uint16_t _y) : x(_x), y(_y) {}
        bool        isValid() const { return x < GRID_SIZE && y < GRID_SIZE; }
        int         get1D()   const { return x * GRID_SIZE + y; }
        int         dist(GridCell2d other) const { return abs(x - other.x) + abs(y - other.y); }
        static GridCell2d from1D(int16_t cell1D) { int16_t x = cell1D / GRID_SIZE; return GridCell2d(x, cell1D - (x*GRID_SIZE)); }
    };
    struct GridOrderPred  // used to sort search order list by dist to the original cell
    {
        GridCell2d searchCell;
        GridOrderPred(uint16_t _idx) : searchCell(GridCell2d::from1D(_idx)) {}
        bool operator() (uint16_t a, uint16_t b) const;
    };


    void        gridInsert(uint16_t idx);
    void        gridRemove(uint16_t idx, int cell);
    void        gridMove(uint16_t idx, const Vector2& oldPos);
    int         getGridCell(Vector2 const& pos);
};

STATIC_ASSERT( sizeof(WormsApp) <= 1024*1024 );
