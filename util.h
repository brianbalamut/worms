#pragma once
#include <math.h>
#include <assert.h>
#include <intrin.h>
#include <stdint.h>

#ifdef _DEBUG
#define ASSERT(cond)  { if( !(cond) ) { __debugbreak(); assert(cond); } }
#else
#define ASSERT(cond)  ((void)0)
#endif

#define STATIC_ASSERT(exp)    typedef char __compile_time_assert[(exp) ? 1 : -1]

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

inline float cross2d(Vector2 const& v1, Vector2 const& v2)
{
    return (v1.x*v2.y) - (v1.y*v2.x);
}

inline Vector2 perp2d(Vector2 const& v1)
{
    return Vector2(v1.y, -v1.x);
}

inline float sign(float f)
{
    return f < 0.0f ? -1.0f : 1.0f;
}

//-- Critically damped spring smoothing -- from Game Programming Gems 4 (1.10)
inline float SmoothSpringCD(float from, float to, float &vel, float const deltaTime, float smoothTime)
{
    float omega = 2.0f / smoothTime;
    float x = omega * deltaTime;
    float exp = 1.0f / (1.0f + x + 0.48f*x*x + 0.235f*x*x*x);
    float change = from - to;
    float temp = (vel + omega * change) * deltaTime;
    vel = (vel - omega * temp) * exp;
    return to + (change + temp) * exp;
}
