// Custom math lib header 
// Some of these things are in c++17 but I can't be arsed to upgrade the project to that so w/e

#pragma once
#include <algorithm>

// Clamps i between the values of min and max
inline float clamp(float i, float min_value, float max_value) { return max(min_value, min(i, max_value)); };

// Degree to radians fuction becuase DirectX uses the former but we're all used to the latter
inline float degreeToRads(float degree) { return (degree * 3.14159f) / 180.f; };

// LERPLERPELERPLERPLERPLERP from a to b by i percent (i goes 0 to 1 and BEYOND FOR EXTRA SCALINGINGGNGNggg)
inline float lerp(float i, float a, float b) { return a + i * (b - a); };