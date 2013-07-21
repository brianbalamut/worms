#include "worms.h"
#include "freeglut.h"

const float kAccelRateTurning = 250.0f;
const float kAccelRateDirect  = 500.0f;
const float kMaxVel           = 175.0f;
const float kExplosionTime    = 1.5f;
const float kExplosionDamp    = 0.995f;
const float kSnapThresholdSq  = 6.0f*6.0f;

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

bool WormsApp::updateHeads(float deltaTime)
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
            return true;

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
                accel *= kAccelRateDirect;
            }
            else
            {
                // find if left or right, turn in that dir
                float signToTarget = sign(cross2d(p.vel, posToTarget));
                accel = perp2d(p.vel).Normalized();
                accel *= kAccelRateTurning * signToTarget;
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

    return false;
}

//------------------------------------------------------------------------------

void WormsApp::updateTails(float deltaTime)
{
    // update pass2 - worm segments chase the position of their next attached segment (going from tail -> head)
    for( int i=0; i < MAX_NUM_PARTICLES; ++i )
    {
        Particle& p = m_particles[i];
        if( p.prevSegment == -1 && p.nextSegment != -1 ) // tail of a worm (of len > 1)
        {
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
        }
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
