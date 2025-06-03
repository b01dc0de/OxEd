#include "OxEd.h" // OxEd.h includes Math.h

float Dot(v2f Left, v2f Right)
{
    return (Left.X * Right.X) + (Left.Y * Right.Y);
}
float Dot(v3f Left, v3f Right)
{
    return (Left.X * Right.X) + (Left.Y * Right.Y) + (Left.Z * Right.Z);
}
float Dot(v4f Left, v4f Right)
{
    return (Left.X * Right.X) + (Left.Y * Right.Y) + (Left.Z * Right.Z) + (Left.W * Right.W);
}

v3f Cross(v3f Left, v3f Right)
{
    v3f Result;
    Result.X = (Left.Y * Right.Z) - (Left.Z * Right.Y);
    Result.Y = (Left.X * Right.Z) - (Left.Z * Right.X);
    Result.Z = (Left.X * Right.Y) - (Left.Y * Right.X);
    return Result;
}

m2f Mult(m2f Left, m2f Right)
{
    m2f Result;
    Result.r0.X = Left.r0.X * Right.r0.X + Left.r0.Y * Right.r1.X;
    Result.r0.Y = Left.r0.X * Right.r0.Y + Left.r0.Y * Right.r1.Y;
    Result.r1.X = Left.r1.X * Right.r0.X + Left.r1.Y * Right.r1.X;
    Result.r1.Y = Left.r1.X * Right.r0.Y + Left.r1.Y * Right.r1.Y;
    return Result;
}

m3f Mult(m3f Left, m3f Right)
{
    m3f Result;
    Result.r0.X = Left.r0.X * Right.r0.X + Left.r0.Y * Right.r1.X + Left.r0.Z * Right.r2.X;
    Result.r0.Y = Left.r0.X * Right.r0.Y + Left.r0.Y * Right.r1.Y + Left.r0.Z * Right.r2.Y;
    Result.r0.Z = Left.r0.X * Right.r0.Z + Left.r0.Y * Right.r1.Z + Left.r0.Z * Right.r2.Z;
    Result.r1.X = Left.r1.X * Right.r0.X + Left.r1.Y * Right.r1.X + Left.r1.Z * Right.r2.X;
    Result.r1.Y = Left.r1.X * Right.r0.Y + Left.r1.Y * Right.r1.Y + Left.r1.Z * Right.r2.Y;
    Result.r1.Z = Left.r1.X * Right.r0.Z + Left.r1.Y * Right.r1.Z + Left.r1.Z + Right.r2.Z;
    Result.r2.X = Left.r2.X * Right.r0.X + Left.r2.Y * Right.r1.X + Left.r2.Z * Right.r2.X;
    Result.r2.Y = Left.r2.X * Right.r0.Y + Left.r2.Y * Right.r1.Y + Left.r2.Z * Right.r2.Y;
    Result.r2.Z = Left.r2.X * Right.r0.Z + Left.r2.Y * Right.r1.Z + Left.r2.Z * Right.r2.Z;
    return Result;
}

m4f Mult(m4f Left, m4f Right)
{
    m4f Result;
    Result.r0.X = Left.r0.X * Right.r0.X + Left.r0.Y * Right.r1.X + Left.r0.Z * Right.r2.X + Left.r0.W * Right.r3.X;
    Result.r0.Y = Left.r0.X * Right.r0.Y + Left.r0.Y * Right.r1.Y + Left.r0.Z * Right.r2.Y + Left.r0.W * Right.r3.Y;
    Result.r0.Z = Left.r0.X * Right.r0.Z + Left.r0.Y * Right.r1.Z + Left.r0.Z * Right.r2.Z + Left.r0.W * Right.r3.Z;
    Result.r0.W = Left.r0.X * Right.r0.W + Left.r0.Y * Right.r1.W + Left.r0.Z * Right.r2.W + Left.r0.W * Right.r3.W;

    Result.r1.X = Left.r1.X * Right.r0.X + Left.r1.Y * Right.r1.X + Left.r1.Z * Right.r2.X + Left.r1.W * Right.r3.X;
    Result.r1.Y = Left.r1.X * Right.r0.Y + Left.r1.Y * Right.r1.Y + Left.r1.Z * Right.r2.Y + Left.r1.W * Right.r3.Y;
    Result.r1.Z = Left.r1.X * Right.r0.Z + Left.r1.Y * Right.r1.Z + Left.r1.Z + Right.r2.Z + Left.r1.W * Right.r3.Z;
    Result.r1.W = Left.r1.X * Right.r0.W + Left.r1.Y * Right.r1.W + Left.r1.Z * Right.r2.W + Left.r1.W * Right.r3.W;

    Result.r2.X = Left.r2.X * Right.r0.X + Left.r2.Y * Right.r1.X + Left.r2.Z * Right.r2.X + Left.r2.W * Right.r3.X;
    Result.r2.Y = Left.r2.X * Right.r0.Y + Left.r2.Y * Right.r1.Y + Left.r2.Z * Right.r2.Y + Left.r2.W * Right.r3.Y;
    Result.r2.Z = Left.r2.X * Right.r0.Z + Left.r2.Y * Right.r1.Z + Left.r2.Z * Right.r2.Z + Left.r2.W * Right.r3.Z;
    Result.r2.W = Left.r2.X * Right.r0.W + Left.r2.Y * Right.r2.W + Left.r2.Z * Right.r2.W + Left.r2.W * Right.r3.W;

    Result.r3.X = Left.r3.X * Right.r0.X + Left.r3.Y * Right.r1.X + Left.r3.Z * Right.r2.X + Left.r3.W * Right.r3.X;
    Result.r3.Y = Left.r3.X * Right.r0.Y + Left.r3.Y * Right.r1.Y + Left.r3.Z * Right.r2.Y + Left.r3.W * Right.r3.Y;
    Result.r3.Z = Left.r3.X * Right.r0.Z + Left.r3.Y * Right.r1.Z + Left.r3.Z * Right.r2.Z + Left.r3.W * Right.r3.Z;
    Result.r3.W = Left.r3.X * Right.r0.W + Left.r3.Y * Right.r2.W + Left.r3.Z * Right.r2.W + Left.r3.W * Right.r3.W;
    return Result;
}

