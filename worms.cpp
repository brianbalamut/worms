#include "worms.h"
#include "freeglut.h"
#include <math.h>

const float kAccelRate       = 150.0f;
const float kMaxVel          = 350.0f;
const float kSnapThresholdSq = 4.0f*4.0f;

//------------------------------------------------------------------------------

float vec2distSq(float x0, float y0, float x1, float y1)
{
    return (x1-x0)*(x1-x0) + (y1-y0)*(y1-y0);
}

float clampf(float value, float min, float max)
{
    return (value < min ? min : (value > max ? max : value));
}

//------------------------------------------------------------------------------

WormsApp::WormsApp()
{
    for( int i=0; i < MAX_NUM_PARTICLES; ++i )
    {
        Particle& p = particles[i];
        p.pos_x = (float)(rand() % SCREEN_WIDTH);
        p.pos_y = (float)(rand() % SCREEN_HEIGHT);
        p.vel_x = 0.0f;
        p.vel_y = 0.0f;

        p.nextSegment = -1;
        p.prevSegment = -1;
        p.wormId = i;
    }
}

//------------------------------------------------------------------------------

void WormsApp::update(float deltaTime)
{
    // update pass1 - worm heads seek towards nearest tails, and possibly attach
    for( int i=0; i < MAX_NUM_PARTICLES; ++i )
    {
        Particle& p = particles[i];
        float accel_x, accel_y;
        int targetIdx = -1;

        if( p.nextSegment == -1 ) // head of a worm
        {
            // if head, find nearest tail as target
            float nearestDistSq = 1e14f;
            for( int i2=0; i2 < MAX_NUM_PARTICLES; ++i2 )  // n^2 lame-o search
            {
                Particle& p2 = particles[i2];
                if( p2.prevSegment != -1 )  // only target tails
                    continue;
                if( p2.wormId == p.wormId ) // skip segments of the same worm (including self)
                    continue;

                float distSq = vec2distSq(p.pos_x, p.pos_y, p2.pos_x, p2.pos_y);
                if( distSq < nearestDistSq )
                {
                    // if within range, just connect
                    if( distSq <= kSnapThresholdSq )
                    {
                        targetIdx = i2;
                        p.nextSegment = i2;
                        p2.prevSegment = i;
                        for( int seg = i; seg != -1; seg = particles[seg].prevSegment ) // fixup wormIds
                            particles[seg].wormId = p2.wormId;
                        break;
                    }
                    else
                    {
                        nearestDistSq = distSq;
                        targetIdx = i2;
                    }
                }
            }

            const Particle& targetP = particles[targetIdx];
            // seek towards target -- should normalize and apply fixed accel rate
            float x0 = sqrtf(vec2distSq(p.pos_x, p.pos_y, targetP.pos_x, targetP.pos_y));
            accel_x = (targetP.pos_x - p.pos_x) / x0;
            accel_y = (targetP.pos_y - p.pos_y) / x0;
            accel_x *= kAccelRate;
            accel_y *= kAccelRate;

            // symplectic integration
            p.vel_x += accel_x * deltaTime;
            p.vel_y += accel_y * deltaTime;
            p.pos_x += p.vel_x * deltaTime;
            p.pos_y += p.vel_y * deltaTime;

            // clamp vel
            p.vel_x = clampf(p.vel_x, -kMaxVel, kMaxVel);
            p.vel_y = clampf(p.vel_y, -kMaxVel, kMaxVel);
        }
    }

    // update pass2 - worm segments inherit the position of their next attached segment (starting from tail -> head)
    for( int i=0; i < MAX_NUM_PARTICLES; ++i )
    {
        Particle& p = particles[i];
        if( p.prevSegment == -1 && p.nextSegment != -1 ) // tail of a worm (of len > 1)
        {
            // inherit next segment's position
            for( int segCur = i, segNext = p.nextSegment; segNext != -1; segCur = segNext, segNext = particles[segNext].nextSegment )
            {
                Particle& pCur  = particles[segCur];
                Particle& pNext = particles[segNext];
                pCur.pos_x = pNext.pos_x;
                pCur.pos_y = pNext.pos_y;
            }
        }
    }
}

//------------------------------------------------------------------------------

void WormsApp::render()
{
    glBegin(GL_POINTS);
    for( int i=0; i < MAX_NUM_PARTICLES; ++i )
    {
        glVertex2f(particles[i].pos_x, particles[i].pos_y);
    }
    glEnd();
}
