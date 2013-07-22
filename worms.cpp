#include "worms.h"
#include "freeglut.h"

const float kAccelRateTurning = 250.0f;
const float kAccelRateDirect  = 500.0f;
const float kMaxVel           = 225.0f;
const float kExplosionTime    = 2.3f;
const float kExplosionDamp    = 0.98f;
const float kSnapThresholdSq  = 4.0f*4.0f;

//------------------------------------------------------------------------------

WormsApp::WormsApp()
{
    reset(true, false);
    setState(kUS_Normal);
}

//------------------------------------------------------------------------------

void WormsApp::reset(bool randPos, bool randVel)
{
    for( int i=0; i < MAX_NUM_PARTICLES; ++i )
    {
        Particle& p = m_particles[i];
        p.nextInGrid  = -1;
        p.nextSegment = -1;
        p.prevSegment = -1;
        p.wormId = i;

        if( randPos )
        {
            p.pos.x = (float)(rand() % SCREEN_WIDTH);
            p.pos.y = (float)(rand() % SCREEN_HEIGHT);
        }

        if( randVel )
        {
            p.vel.x = (float)(rand() % (int)kMaxVel*2.0f) - kMaxVel;
            p.vel.y = (float)(rand() % (int)kMaxVel*2.0f) - kMaxVel;
        }
        else
        {
            p.vel.x = 0.0f;
            p.vel.y = 0.0f;
        }
    }

    // rebuild grid
    for( int i=0; i < GRID_COUNT; ++i )
        m_grid[i] = -1;
    for( int i=0; i < MAX_NUM_PARTICLES; ++i )
        gridInsert(i);

    memset(m_tailFlags, 0xFF, sizeof(m_tailFlags));
}

//------------------------------------------------------------------------------

void WormsApp::update(float deltaTime)
{
    if( m_state == kUS_Normal )
    {
        bool lastWorm = updateHeads(deltaTime);
        if( lastWorm )
            setState(kUS_LastWorm);
        updateTails(deltaTime);
    }
    else if( m_state == kUS_LastWorm )
    {
        reset(false, true); // keep pos, rand vel
        setState(kUS_Exploding);
    }
    else if( m_state == kUS_Exploding )
    {
        updateExploding(deltaTime);
        if( m_stateTime >= kExplosionTime )
        {
            reset(false, false); // keep pos, clear vel
            setState(kUS_Normal);
        }
    }

    m_stateTime += deltaTime;
}

//------------------------------------------------------------------------------

void WormsApp::render()
{
    glBegin(GL_POINTS);
    for( int i=0; i < MAX_NUM_PARTICLES; ++i )
    {
        glVertex2f(m_particles[i].pos.x, m_particles[i].pos.y);
    }
    glEnd();
}

//------------------------------------------------------------------------------

int WormsApp::getTimerMs() const
{
    if( m_state == kUS_Exploding )
        return 1000/SCREEN_FPS;
    static bool s_fpsCapped = false;
    return s_fpsCapped ? 1000/SCREEN_FPS : 0;
}

//------------------------------------------------------------------------------

int WormsApp::getNearestTail(Particle const& p, float * pDistSq)
{
    int nearestIdx = -1;
    float nearestDistSq = 1e14f;
    int cell = getGridCell(p.pos);
    int startCell = cell;

    while( nearestIdx == -1 )
    {
        for( int idx = m_grid[cell]; idx != -1; idx = m_particles[idx].nextInGrid )
        {
            Particle& p2 = m_particles[idx];
            if( !p2.isTail() )
                continue;
            if( p2.wormId == p.wormId ) // skip segments of the same worm (including self)
                continue;

            float distSq = p.pos.DistSq(p2.pos);
            if( distSq < nearestDistSq )
            {
                nearestIdx = idx;
                nearestDistSq = distSq;

                if( distSq <= kSnapThresholdSq )
                    break;
            }
        }
        cell = (cell+1) % GRID_COUNT;  // just trying the other grids in memory order... so only closest for the in-grid ones! finding the right grid search order seems hard to think about for now
        if( startCell == cell )
            break; // we wrapped
    }

    if( pDistSq != NULL )
        *pDistSq = nearestDistSq;
    return nearestIdx;
}

//------------------------------------------------------------------------------

bool WormsApp::updateHeads(float deltaTime)
{
    // update pass1 - worm heads seek towards nearest tails, and possibly attach
    for( int i=0; i < MAX_NUM_PARTICLES; ++i )
    {
        Particle& p = m_particles[i];
        if( !p.isHead() )
            continue;

        float distSq = 0.0f;
        int targetIdx = getNearestTail(p, &distSq);

        if( targetIdx == -1 ) // couldnt find target, must be the last worm!
            return true;

        // if within range, just connect
        if( distSq <= kSnapThresholdSq )
        {
            Particle& p2 = m_particles[targetIdx];
            p.nextSegment = targetIdx;
            p2.prevSegment = i;
            p.vel.x = p.vel.y = 0.0f;
            for( int seg = i; seg != -1; seg = m_particles[seg].prevSegment ) // fixup wormIds
                m_particles[seg].wormId = p2.wormId;

            // clear flag of the tail we're chomping
            uint32_t flagIdx = targetIdx / 32;
            uint32_t flagOffset = targetIdx % 32;
            m_tailFlags[flagIdx] &= ~(1UL<<flagOffset);

            gridRemove(targetIdx, getGridCell(p2.pos));  // stop tracking the former tail
        }
        else
        {
            // seek towards target
            Vector2 accel;
            Vector2 targetPos = m_particles[targetIdx].pos;
            Vector2 posToTarget = targetPos - p.pos;
            float dot = p.vel.Dot(posToTarget.Normalized());
            if( dot <= 0.0f || dot > 0.9f ) // target is behind us, or approx in front of us
            {
                // accelerate directly towards target at constant rate
                accel = (targetPos - p.pos).Normalized();
                accel *= kAccelRateDirect;
            }
            else
            {
                // find if left or right, turn in that dir
                float signToTarget = sign(cross2d(p.vel, posToTarget));
                accel = perp2d(p.vel).Normalized();
                accel *= kAccelRateTurning * signToTarget;
            }

            Vector2 oldPos = p.pos;

            // symplectic integration
            p.vel += accel * deltaTime;
            p.pos += p.vel * deltaTime;

            // clamp vel
            p.vel.x = clampf(p.vel.x, -kMaxVel, kMaxVel);
            p.vel.y = clampf(p.vel.y, -kMaxVel, kMaxVel);
            screenCollide(p);

            if( p.isTail() )
                gridMove(i, oldPos);
        }
    }

    return false;
}

//------------------------------------------------------------------------------

void WormsApp::updateTails(float deltaTime)
{
    // update pass2 - worm segments chase the position of their next attached segment (going from tail -> head)
    for( int i=0; i < MAX_NUM_PARTICLES; ++i )
    {
        Particle& p = m_particles[i];
        if( p.isTail() && p.nextSegment != -1 ) // tail of a worm (of len > 1)
        {
            Vector2 oldPos = p.pos;

            // chase next segment's position
            for( int segCur = i, segNext = p.nextSegment; segNext != -1; segCur = segNext, segNext = m_particles[segNext].nextSegment )
            {
                Particle& pCur  = m_particles[segCur];
                Particle& pNext = m_particles[segNext];

                float dist = pCur.pos.DistSq(pNext.pos);
                if( dist < squared(3.5f) )
                {
                    pCur.pos = pNext.pos;
                    pCur.vel = pNext.vel;
                }
                else
                {
                    float smoothTime = 0.03f;
                    SmoothSpringCD(pCur.pos.x, pNext.pos.x, pCur.vel.x, deltaTime, smoothTime);
                    SmoothSpringCD(pCur.pos.y, pNext.pos.y, pCur.vel.y, deltaTime, smoothTime);
                    pCur.pos += pCur.vel * deltaTime;

                    // clamp vel
                    //pCur.vel.x = clampf(pCur.vel.x, -kMaxVel, kMaxVel);
                    //pCur.vel.y = clampf(pCur.vel.y, -kMaxVel, kMaxVel);
                }
            }

            gridMove(i, oldPos); // only move tails, not mid segments
        }
    }
}

//------------------------------------------------------------------------------

void WormsApp::screenCollide(Particle& p)
{
    // clamp pos, reflect vel on collide
    if( p.pos.x < 1.0f || p.pos.x >= (float)SCREEN_WIDTH )
    {
        p.pos.x = clampf(p.pos.x, 1.0f, (float)SCREEN_WIDTH-1.0f);
        p.vel.x *= -1.0f;
    }
    if( p.pos.y < 1.0f || p.pos.y >= (float)SCREEN_HEIGHT )
    {
        p.pos.y = clampf(p.pos.y, 1.0f, (float)SCREEN_HEIGHT-1.0f);
        p.vel.y *= -1.0f;
    }
}

//------------------------------------------------------------------------------

void WormsApp::updateExploding(float deltaTime)
{
    for( int i=0; i < MAX_NUM_PARTICLES; ++i )
    {
        Particle& p = m_particles[i];
        p.pos += p.vel * deltaTime;
        p.vel *= kExplosionDamp;

        screenCollide(p);
    }
}

//------------------------------------------------------------------------------

void WormsApp::gridInsert(int idx)
{
    Particle& p = m_particles[idx];
    int cell = getGridCell(p.pos);
    p.nextInGrid = m_grid[cell];
    m_grid[cell] = idx;
}

//------------------------------------------------------------------------------

void WormsApp::gridRemove(int idx, int cell)
{
    Particle& p = m_particles[idx];

    // handle special case: head of list
    if( m_grid[cell] == idx )
    {
        m_grid[cell] = p.nextInGrid;
        return;
    }

    for( int i = m_grid[cell]; i != -1; )
    {
        Particle& gridP = m_particles[i];
        i = gridP.nextInGrid; // increment
        if( i == idx )
        {
            gridP.nextInGrid = p.nextInGrid;
            p.nextInGrid = -1;
            return;
        }
    }

    ASSERT(false); // shouldn't reach here!
}

//------------------------------------------------------------------------------

void WormsApp::gridMove(int idx, const Vector2& oldPos)
{
    Particle& p = m_particles[idx];
    int curCell = getGridCell(oldPos);
    int newCell = getGridCell(p.pos);
    if( newCell != curCell )
    {
        gridRemove(idx, curCell);
        gridInsert(idx);
    }
}

//------------------------------------------------------------------------------

int WormsApp::getGridCell(Vector2 const& pos)
{
    uint32_t x = static_cast<uint32_t>(pos.x) / (SCREEN_WIDTH  / GRID_SIZE);
    uint32_t y = static_cast<uint32_t>(pos.y) / (SCREEN_HEIGHT / GRID_SIZE);
    ASSERT( x < GRID_SIZE && y < GRID_SIZE );
    return x*GRID_SIZE + y;
}
