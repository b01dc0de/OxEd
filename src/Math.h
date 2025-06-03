#ifndef MATH_H
#define MATH_H

struct v2f
{
    float X = 0.0f;
    float Y = 0.0f;
};
struct v3f
{
    float X = 0.0f;
    float Y = 0.0f;
    float Z = 0.0f;
};
struct v4f
{
    float X = 0.0f;
    float Y = 0.0f;
    float Z = 0.0f;
    float W = 0.0f;
};

float Dot(v2f Left, v2f Right);
float Dot(v3f Left, v3f Right);
float Dot(v4f Left, v4f Right);
v3f Cross(v3f Left, v3f Right);

struct m2f
{
    v2f r0;
    v2f r1;
};
struct m3f
{
    v3f r0;
    v3f r1;
    v3f r2;
};
struct m4f
{
    v4f r0;
    v4f r1;
    v4f r2;
    v4f r3;
};

m2f Mult(m2f Left, m2f Right);
m3f Mult(m3f Left, m3f Right);
m4f Mult(m4f Left, m4f Right);

#endif // MATH_H

