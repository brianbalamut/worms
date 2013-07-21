#pragma once
#include <math.h>

//------------------------------------------------------------------------------

const int   SCREEN_WIDTH      = 800;
const int   SCREEN_HEIGHT     = 600;
const int   SCREEN_FPS        = 30;
const float POINT_SIZE        = 2;
const int   MAX_NUM_PARTICLES = 1024;

//------------------------------------------------------------------------------

class Vector2 {
public:
    float x, y;

    Vector2() {}
    Vector2(float _x, float _y) : x(_x), y(_y) {}
    Vector2(const Vector2 &v) : x(v.x), y(v.y) {}

    Vector2 operator+(const Vector2 &v) const { return Vector2(x+v.x, y+v.y); }
    Vector2 operator-(const Vector2 &v) const { return Vector2(x-v.x, y-v.y); }
    Vector2 operator*(float s) const { return Vector2(x*s, y*s); }
    Vector2 operator/(float s) const { return Vector2(x/s, y/s); }
    void operator+=(const Vector2 &v) { x+=v.x; y+=v.y; }
    void operator*=(float s) { x*=s; y*=s; }
    bool operator==(const Vector2 &v) const { return x==v.x && y==v.y; }

    Vector2 Normalized() const { return *this / Length(); } // unsafe
    float Dot(const Vector2 &v) const { return x*v.x+y*v.y; }
    float LengthSq() const { return Dot(*this); }
    float Length() const { return sqrtf(LengthSq()); }
    float DistSq(const Vector2 &v) const { return (*this - v).LengthSq(); }
    float Dist(const Vector2 &v) const { return sqrtf(DistSq(v)); }
};

inline float clampf(float value, float min, float max)
{
    return (value < min ? min : (value > max ? max : value));
}

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
    Particle particles[MAX_NUM_PARTICLES];

public:
    WormsApp();

    void update(float deltaTime);
    void render();
};

//static_assert( sizeof(WormsApp) <= 1024*1024 );
