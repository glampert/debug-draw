
// ================================================================================================
// -*- C++ -*-
// File:   vectormath.h
// Author: Guilherme R. Lampert
// Brief:  This header exposes the Sony Vector Math library types into the global scope.
// ================================================================================================

#ifndef VECTORMATH_H_
#define VECTORMATH_H_

// We're using the generic Array of Structures (AoS) format.
#include "cpp/vectormath_aos.h"
using namespace Vectormath::Aos;

inline float * toFloatPtr(Point3  & p) { return reinterpret_cast<float *>(&p); }
inline float * toFloatPtr(Vector3 & v) { return reinterpret_cast<float *>(&v); }
inline float * toFloatPtr(Vector4 & v) { return reinterpret_cast<float *>(&v); }
inline float * toFloatPtr(Quat    & q) { return reinterpret_cast<float *>(&q); }
inline float * toFloatPtr(Matrix3 & m) { return reinterpret_cast<float *>(&m); }
inline float * toFloatPtr(Matrix4 & m) { return reinterpret_cast<float *>(&m); }

inline const float * toFloatPtr(const Point3  & p) { return reinterpret_cast<const float *>(&p); }
inline const float * toFloatPtr(const Vector3 & v) { return reinterpret_cast<const float *>(&v); }
inline const float * toFloatPtr(const Vector4 & v) { return reinterpret_cast<const float *>(&v); }
inline const float * toFloatPtr(const Quat    & q) { return reinterpret_cast<const float *>(&q); }
inline const float * toFloatPtr(const Matrix3 & m) { return reinterpret_cast<const float *>(&m); }
inline const float * toFloatPtr(const Matrix4 & m) { return reinterpret_cast<const float *>(&m); }

// Shorthand to discard the last element of a Vector4 and get a Point3.
inline Point3 toPoint3(const Vector4 & v4)
{
    return Point3(v4[0], v4[1], v4[2]);
}

// Convert from world (global) coordinates to local model coordinates.
// Input matrix must be the inverse of the model matrix, e.g.: 'inverse(modelMatrix)'.
inline Point3 worldPointToModel(const Matrix4 & invModelToWorldMatrix, const Point3 & point)
{
    return toPoint3(invModelToWorldMatrix * point);
}

#endif // VECTORMATH_H_
