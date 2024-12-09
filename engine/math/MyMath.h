#pragma once
#include "Matrix4x4.h"
#include "Matrix3x3.h"
#include "Vector4.h"
#include "Vector3.h"
#include "Vector2.h"
#include <assert.h>
#include <cmath>
#include <stdio.h>
#include <vector>
#include <string>


Matrix4x4 MakeTranslateMatrix(const Vector3& translate);

Matrix4x4 MakeScaleMatrix(const Vector3& scale);

Vector3 TransformVector3(const Vector3& vector, const Matrix4x4& matrix);

Matrix4x4 MakeRotateXMatrix(float radian);

Matrix4x4 MakeRotateYMatrix(float radian);

Matrix4x4 MakeRotateZMatrix(float radian);

Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate);

Matrix4x4 Add(const Matrix4x4& mt1, const Matrix4x4& mt2);

Matrix4x4 Subtract(const Matrix4x4& mt1, const Matrix4x4& mt2);

Matrix4x4 Multiply(const Matrix4x4& mt1, const Matrix4x4& mt2);

Matrix4x4 Inverse(const Matrix4x4& m);

Matrix4x4 Transpose(const Matrix4x4& mt1);

Matrix4x4 MakeIdentity4x4();

Vector3 Cross(const Vector3& v1, const Vector3& v2);