#ifndef _STRUCTS_H
#define _STRUCTS_H

#define Vc1 float
#define Vc2 float2
#define Vc3 Stride3f
#define Vc4 float4
#define Vc4x4 float4x4
#define iVc1 int
#define iVc2 int2
#define iVc3 Stride3i
#define iVc4 int4

struct Stride4f {
    float x;
    float y;
    float z;
    float w;
};

struct Stride4i {
    int x;
    int y;
    int z;
    int w;
};

struct Stride2f {
    float x;
    float y;
};

struct Stride3f {
    float x;
    float y;
    float z;
};

struct Stride3i {
    int x;
    int y;
    int z;
};

float2 toVec2(in Stride2f a){
    return float2(a.x, a.y);
}

float3 toVec3(in Stride3f a){
    return float3(a.x, a.y, a.z);
}

float4 toVec4(in Stride4f a){
    return float4(a.x, a.y, a.z, a.w);
}

int3 toVec3(in Stride3i a){
    return int3(a.x, a.y, a.z);
}

Stride2f toStride2(in float2 a){
    Stride2f o;
    o.x = a.x;
    o.y = a.y;
    return o;
}

Stride3f toStride3(in float3 a){
    Stride3f o;
    o.x = a.x;
    o.y = a.y;
    o.z = a.z;
    return o;
}

Stride4f toStride4(in float4 a){
    Stride4f o;
    o.x = a.x;
    o.y = a.y;
    o.z = a.z;
    o.w = a.w;
    return o;
}

Stride4i toStride4(in int4 a){
    Stride4i o;
    o.x = a.x;
    o.y = a.y;
    o.z = a.z;
    o.w = a.w;
    return o;
}

struct Texel {
    Vc4 coord;
    /*
#ifdef ENABLE_NVIDIA_INSTRUCTION_SET
    Vc4 samplecolor;
#else
    iVc4 samplecolor;
#endif
*/
    iVc4 EXT;
};

struct bbox {
#ifdef USE_WARP_OPTIMIZED
    Vc1 mn[4];
    Vc1 mx[4];
#else
    Vc4 mn;
    Vc4 mx;
#endif
};

struct Ray {
#ifdef USE_WARP_OPTIMIZED
    Vc1 origin[4];
    Vc1 direct[4];
#else
    Vc4 origin;
    Vc4 direct;
#endif
    Vc4 color;
    Vc4 final;
    iVc4 params;
    iVc1 idx;
    iVc1 bounce;
    iVc1 texel;
    iVc1 actived;
};

struct Hit {
    Vc4 normal;
    Vc4 tangent;
    Vc4 texcoord;
    Vc4 vcolor;
    Vc4 vmods;
    Vc1 dist;
    iVc1 triangleID;
    iVc1 materialID;
    iVc1 shaded;
};

struct Leaf {
    bbox box;
    iVc4 pdata;
};

struct HlbvhNode {
    bbox box;
#ifdef USE_WARP_OPTIMIZED
    iVc1 pdata[4];
#else
    iVc4 pdata;
#endif
    iVc4 leading;
};

struct VboDataStride {
    Vc4 vertex;
    Vc4 normal;
    Vc4 texcoord;
    Vc4 color;
    Vc4 modifiers;
};

struct ColorChain {
    Vc4 color;
    iVc4 cdata;
};

#endif