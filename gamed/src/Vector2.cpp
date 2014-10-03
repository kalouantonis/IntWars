// Vector2.cpp

#include "stdafx.h"
#include "Vector2.h"

/*
// note: this function is not a member function!
// Commented out because they're replaced
Vector2 operator-(const Vector2 &c1, const Vector2 &c2)
{
   // use the Cents constructor and operator+(int, int)
   return Vector2(c1.X-c2.X, c1.Y-c2.Y);
}

 Vector2 operator+(const Vector2 &c1, const Vector2 &c2)
 {
    // use the Cents constructor and operator+(int, int)
    return Vector2(c1.X + c2.X, c1.Y + c2.Y);
 }

 Vector2 operator*(const Vector2 &c1, const int c2)
 {
    // use the Cents constructor and operator+(int, int)
    return Vector2(c1.X * c2, c1.Y * c2);
 }

 Vector2 operator*(const Vector2 &c1, const double c2)
 {
    // use the Cents constructor and operator+(int, int)
    return Vector2(c1.X * c2, c1.Y * c2);
 }*/


// Returns the length of the vector
float Vector2::Length(){
   return sqrt(X * X + Y * Y);
}

// Returns the length of the vector
float Vector2::SqrLength(){
   return (X * X + Y * Y);
}

// Normalizes the vector
Vector2 Vector2::Normalize(){
   Vector2 vector;
   float length = this->Length();

   if (length != 0){
      vector.X = X / length;
      vector.Y = Y / length;
   }

   return vector;
}

Vector2::~Vector2(void)
{
}