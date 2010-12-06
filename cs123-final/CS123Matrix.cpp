/*!
   @file   CS123Matrix.cpp
   @author Travis Fischer (fisch0920@gmail.com)
   @date   Fall 2008
   
   @brief
      Provides basic functionality for a templated, arbitrarily-sized matrix.
      You will need to fill this file in for the Camtrans assignment.

**/

// CS123Matrix.h can be found in /course/cs123/include/algebra
#include <CS123Matrix.h>
#include <iostream>

//@name Routines which generate specific-purpose transformation matrices
//@{---------------------------------------------------------------------
// @returns the scale matrix described by the vector
Matrix4x4 getScaleMat(const Vector4 &v) {


    return Matrix4x4(v.x, 0, 0, 0,
                     0, v.y, 0, 0,
                     0, 0, v.z, 0,
                     0, 0, 0, 1);

}

// @returns the translation matrix described by the vector
Matrix4x4 getTransMat(const Vector4 &v) {

    return Matrix4x4(1,0,0,v.x,
                     0,1,0,v.y,
                     0,0,1,v.z,
                     0,0,0,1);


}

// @returns the rotation matrix about the x axis by the specified angle
Matrix4x4 getRotXMat (const REAL radians) {

    REAL x = cos(radians);
    REAL y = sin(radians);
    return Matrix4x4(1,0,0,0,
                     0,x, -y,0,
                     0,y, x, 0,
                     0,0,0,1);


}

// @returns the rotation matrix about the y axis by the specified angle
Matrix4x4 getRotYMat (const REAL radians) {
    REAL x = cos(radians);
    REAL y = sin(radians);
    return Matrix4x4(x, 0, y,0,
                     0,1,0,0,
                     -y, 0, x, 0,
                     0,0,0,1);


}

// @returns the rotation matrix about the z axis by the specified angle
Matrix4x4 getRotZMat (const REAL radians) {
    REAL x = cos(radians);
    REAL y = sin(radians);
    return Matrix4x4(x, -y,0,0,
                     y, x, 0,0,
                     0,0,1,0,
                     0,0,0,1);


}

// @returns the rotation matrix around the vector and point by the specified angle
Matrix4x4 getRotMat  (const Vector4 &p, const Vector4 &v, const REAL a) {

    //compute the two angles other than a to rotate by (we are assuming that a is in radians)
    REAL x = atan2(v.z, v.x);
    REAL y = -atan2(v.y, sqrt(v.x*v.x + v.z*v.z));
    //compute the X, Y, and Z matrices for each angle
    Matrix4x4 X = getRotXMat(a);
    Matrix4x4 Y = getRotYMat(x);
    Matrix4x4 Z = getRotZMat(y);
    //then the inverses
    Matrix4x4 invY = getInvRotYMat(x);
    Matrix4x4 invZ = getInvRotZMat(y);
    //and the translation matrix to move the point p to the origin
    Matrix4x4 T = getTransMat(p * -1);
    //and the inverse translation matrix
    Matrix4x4 invT = getInvTransMat(p * -1);
    //get the point to the origin and the axes to line up, and then apply the rotation matrix
    return invT * invY * invZ * X * Z * Y * T;


}


// @returns the inverse scale matrix described by the vector
Matrix4x4 getInvScaleMat(const Vector4 &v) {

    return Matrix4x4(1/v.x, 0,0,0,
                     0,1/v.y,0,0,
                     0,0,1/v.z, 0,
                     0,0,0,1);


}

// @returns the inverse translation matrix described by the vector
Matrix4x4 getInvTransMat(const Vector4 &v) {


    return Matrix4x4(1,0,0,-v.x,
                     0,1,0,-v.y,
                     0,0,1,-v.z,
                     0,0,0,1);


}

// @returns the inverse rotation matrix about the x axis by the specified angle
Matrix4x4 getInvRotXMat (const REAL radians) {

    REAL x = cos(radians);
    REAL y = sin(radians);
    return Matrix4x4(1,0,0,0,
                     0,x,y,0,
                     0,-y,x,0,
                     0,0,0,1);


}

// @returns the inverse rotation matrix about the py axis by the specified angle
Matrix4x4 getInvRotYMat (const REAL radians) {
    REAL x = cos(radians);
    REAL y = sin(radians);
    return Matrix4x4(x, 0, -y,0,
                     0,1,0,0,
                     y, 0, x, 0,
                     0,0,0,1);


}

// @returns the inverse rotation matrix about the z axis by the specified angle
Matrix4x4 getInvRotZMat (const REAL radians) {

    REAL x = cos(radians);
    REAL y = sin(radians);
    return Matrix4x4(x, y,0,0,
                     -y, x, 0,0,
                     0,0,1,0,
                     0,0,0,1);


}

// @returns the inverse rotation matrix around the vector and point by the specified angle
Matrix4x4 getInvRotMat  (const Vector4 &p, const Vector4 &v, const REAL a) {

    //This is the same as above, but X is now the inverse X matrix so it undoes the matrix composition from above

    REAL x = atan2(v.z, v.x);
    REAL y = -atan2(v.y, sqrt(v.x*v.x + v.z*v.z));
    Matrix4x4 X = getInvRotXMat(a);
    Matrix4x4 Y = getRotYMat(x);
    Matrix4x4 Z = getRotZMat(y);
    Matrix4x4 invY = getInvRotYMat(x);
    Matrix4x4 invZ = getInvRotZMat(y);
    Matrix4x4 T = getTransMat(p * -1);
    Matrix4x4 invT = getInvTransMat(p * -1);
    return invT * invY * invZ * X * Z * Y * T;


}


//@}---------------------------------------------------------------------

