#include "worms.h"
#include "freeglut.h"

const float kAccelRate       = 750.0f;
const float kMaxVel          = 275.0f;
const float kSnapThresholdSq = 6.0f*6.0f;

//------------------------------------------------------------------------------

WormsApp::WormsApp()
{
    reset(true, false);
    m_state = kUS_Normal;
}

//------------------------------------------------------------------------------

void WormsApp::reset(bool randPos, bool randVel)
{
    for( int i=0; i < MAX_NUM_PARTICLES; ++i )
    {
        Particle& p = m_particles[i];

        if( randPos )
        {
            p.pos.x = (float)(rand() % SCREEN_WIDTH);
            p.pos.y = (float)(rand() % SCREEN_HEIGHT);
        }

        if( randVel )
        {
            p.vel.x = (float)(rand() % (int)kMaxVel) - kMaxVel*0.5f;
            p.vel.y = (float)(rand() % (int)kMaxVel) - kMaxVel*0.5f;
        }
        else
        {
            p.vel.x = 0.0f;
            p.vel.y = 0.0f;
        }

        p.nextSegment = -1;
        p.prevSegment = -1;
        p.wormId = i;
    }
}

//------------------------------------------------------------------------------

void WormsApp::update(float deltaTime)
{
    if( m_state == kUS_Normal )
    {
        m_state = updateHeads(deltaTime);
        updateTails(deltaTime);
    }
    else if( m_state == kUS_LastWorm )
    {
        reset(false, true); // keep pos, rand vel
        m_state = kUS_Exploding;
    }
    else if( m_state == kUS_Exploding )
    {
        m_state = kUS_Normal;
        //updateExploding();
    }
}

//------------------------------------------------------------------------------

WormsApp::UpdateState WormsApp::updateHeads(float deltaTime)
{
    // update pass1 - worm heads seek towards nearest tails, and possibly attach
    for( int i=0; i < MAX_NUM_PARTICLES; ++i )
    {
        Particle& p = m_particles[i];
        int targetIdx = -1;

        if( p.nextSegment != -1 ) // head of a worm
            continue;
        // if head, find nearest tail as target
        float nearestDistSq = 1e14f;
        for( int i2=0; i2 < MAX_NUM_PARTICLES; ++i2 )  // n^2 lame-o search
        {
            Particle& p2 = m_particles[i2];
            if( p2.prevSegment != -1 )  // only target tails
                continue;
            if( p2.wormId == p.wormId ) // skip segments of the same worm (including self)
                continue;

            float distSq = p.pos.DistSq(p2.pos);
            if( distSq < nearestDistSq )
            {
                // if within range, just connect
                if( distSq <= kSnapThresholdSq )
                {
                    targetIdx = i2;
                    p.nextSegment = i2;
                    p2.prevSegment = i;
                    p.vel.x = p.vel.y = 0.0f;
                    for( int seg = i; seg != -1; seg = m_particles[seg].prevSegment ) // fixup wormIds
                        m_particles[seg].wormId = p2.wormId;
                    break;
                }
                else
                {
                    nearestDistSq = distSq;
                    targetIdx = i2;
                }
            }
        }

        if( targetIdx == -1 ) // couldnt find target, must be the last worm!
            return kUS_LastWorm;

        if( p.nextSegment == -1 ) // still unattached
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
                accel *= kAccelRate;
            }
            else
            {
                // find if left or right, turn in that dir
                float signToTarget = sign(cross2d(p.vel, posToTarget));
                accel = perp2d(p.vel).Normalized();
                accel *= kAccelRate * signToTarget;
            }

            // symplectic integration
            p.vel += accel * deltaTime;
            p.pos += p.vel * deltaTime;

            // clamp vel
            p.vel.x = clampf(p.vel.x, -kMaxVel, kMaxVel);
            p.vel.y = clampf(p.vel.y, -kMaxVel, kMaxVel);

            // clamp pos, reflect vel on collide
            if( p.pos.x < 0.0f || p.pos.x > (float)SCREEN_WIDTH )
            {
                p.pos.x = clampf(p.pos.x, 0.0f, (float)SCREEN_WIDTH);
                p.vel.x *= -1.0f;
            }
            if( p.pos.y < 0.0f || p.pos.y > (float)SCREEN_HEIGHT )
            {
                p.pos.y = clampf(p.pos.y, 0.0f, (float)SCREEN_HEIGHT);
                p.vel.y *= -1.0f;
            }
        }
    }

    return kUS_Normal;
}

//------------------------------------------------------------------------------

void WormsApp::updateTails(float deltaTime)
{
    // update pass2 - worm segments inherit the position of their next attached segment (going from tail -> head)
    for( int i=0; i < MAX_NUM_PARTICLES; ++i )
    {
        Particle& p = m_particles[i];
        if( p.prevSegment == -1 && p.nextSegment != -1 ) // tail of a worm (of len > 1)
        {
            // inherit next segment's position
            for( int segCur = i, segNext = p.nextSegment; segNext != -1; segCur = segNext, segNext = m_particles[segNext].nextSegment )
            {
                Particle& pCur  = m_particles[segCur];
                Particle& pNext = m_particles[segNext];

                float smoothTime = 0.1f;
                SmoothSpringCD(pCur.pos.x, pNext.pos.x, pCur.vel.x, deltaTime, smoothTime);
                SmoothSpringCD(pCur.pos.y, pNext.pos.y, pCur.vel.y, deltaTime, smoothTime);

                pCur.pos += pCur.vel * deltaTime;
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
        glVertex2f(m_particles[i].pos.x, m_particles[i].pos.y);
    }
    glEnd();
}
