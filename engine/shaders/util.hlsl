#ifndef __UTIL_HLSL__
#define __UTIL_HLSL__

#pragma pack_matrix(row_major) // This MUST come before all matrix declarations

float maprange(float a1, float b1, float a2, float b2, float input)
{
    float lerp = (input - a1) / (b1 - a1);
    return a2 + (b2 - a2) * lerp;
}

#endif // end include guard