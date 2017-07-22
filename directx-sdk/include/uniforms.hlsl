#ifndef _UNIFORMS_H
#define _UNIFORMS_H

#include "../include/structs.hlsl"

//#define RAY_BLOCK rayBlock[0].
//#define GEOMETRY_BLOCK geometryBlock[0].

//#define RAY_BLOCK 
//#define GEOMETRY_BLOCK 

/*
struct RandomUniformStruct {
    uint time;
    Vc1 _reserved0;
    Vc1 _reserved1;
    Vc1 _reserved2;
};

struct MinmaxUniformStruct {
    iVc1 heap;
    Vc1 prec;
    Vc1 _reserved0;
    Vc1 _reserved1;
};

struct MaterialUniformStruct {
    Vc1 materialID;
    Vc1 f_shadows;
    Vc1 f_reflections;
    Vc1 lightcount;
    Vc4 backgroundColor; // for skybox configure
    iVc4 iModifiers0;
    iVc4 iModifiers1;
    Vc4 fModifiers0;
    Vc4 fModifiers1;
    Vc4x4 transformModifier;
};

struct SamplerUniformStruct {
    Vc2 sceneRes;
    iVc1 samplecount;
    iVc1 rayCount;
    iVc1 iteration;
    iVc1 phase;
    iVc1 maxSamples;
    iVc1 currentSample;
    iVc1 maxFilters;
    iVc1 currentRayLimit;
};

struct GeometryUniformStruct {
    Vc4x4 transform;
    Vc4x4 transformInv;
    Vc4x4 texmatrix;
    Vc4 colormod;

    Vc1 offset;
    iVc1 materialID;
    iVc1 triangleCount;
    iVc1 triangleOffset;

    iVc1 unindexed;
    iVc1 loadOffset;
    iVc1 NB_mode;
    iVc1 clearDepth;
};

struct OctreeUniformStruct {
    Vc4x4 project;
    Vc4x4 unproject;
    Vc4x4 transform;
    Vc4x4 transformInv;

    iVc1 maxDepth;
    iVc1 currentDepth;
    iVc1 nodeCount;
    iVc1 unk0;
};

struct CameraUniformStruct {
    Vc4x4 projInv;
    Vc4x4 camInv;
    Vc4x4 camInv2;

    Vc1 prob;
    iVc1 enable360;
    iVc1 interlace;
    iVc1 interlaceStage;
};

struct AttributeUniformStruct {
    iVc1 vertexOffset;
    iVc1 normalOffset;
    iVc1 texcoordOffset;
    iVc1 lightcoordOffset;

    iVc1 colorOffset;
    iVc1 stride;
    iVc1 mode;
    iVc1 colorFormat;

    iVc1 haveColor;
    iVc1 haveNormal;
    iVc1 haveTexcoord;
    iVc1 haveLightcoord;

    // for future shaders
    iVc4 iModifiers0;
    iVc4 iModifiers1;
    Vc4 fModifiers0;
    Vc4 fModifiers1;
};
*/



struct LightUniformStruct {
    Vc4 lightVector;
    Vc4 lightColor;
    Vc4 lightOffset;
    Vc4 lightAmbient;
};

struct RayBlockUniform {
    //SamplerUniformStruct 
    Vc2 sceneRes;
    iVc1 samplecount;
    iVc1 rayCount;
    iVc1 iteration;
    iVc1 phase;
    iVc1 maxSamples;
    iVc1 currentSample;
    iVc1 maxFilters;
    iVc1 currentRayLimit;

    //CameraUniformStruct
    Vc4x4 projInv;
    Vc4x4 camInv;
    Vc4x4 camInv2;

    Vc1 prob;
    iVc1 enable360;
    iVc1 interlace;
    iVc1 interlaceStage;

    //MaterialUniformStruct
    Vc1 materialID;
    Vc1 f_shadows;
    Vc1 f_reflections;
    Vc1 lightcount;
    Vc4 backgroundColor; // for skybox configure
    iVc4 iModifiers0;
    iVc4 iModifiers1;
    Vc4 fModifiers0;
    Vc4 fModifiers1;
    Vc4x4 transformModifier;

    //RandomUniformStruct
    uint time;
    Vc1 _reserved0;
    Vc1 _reserved1;
    Vc1 _reserved2;

    //SamplerUniformStruct samplerUniform;
    //CameraUniformStruct cameraUniform;
    //MaterialUniformStruct materialUniform;
    //RandomUniformStruct randomUniform;
};

struct GeometryBlockUniform {
    // AttributeUniformStruct
    iVc1 vertexOffset;
    iVc1 normalOffset;
    iVc1 texcoordOffset;
    iVc1 lightcoordOffset;

    iVc1 colorOffset;
    iVc1 stride;
    iVc1 mode;
    iVc1 colorFormat;

    iVc1 haveColor;
    iVc1 haveNormal;
    iVc1 haveTexcoord;
    iVc1 haveLightcoord;

    iVc4 iModifiers0;
    iVc4 iModifiers1;
    Vc4 fModifiers0;
    Vc4 fModifiers1;

    //GeometryUniformStruct
    Vc4x4 transform;
    Vc4x4 transformInv;
    Vc4x4 texmatrix;
    Vc4 colormod;

    Vc1 offset;
    iVc1 materialID;
    iVc1 triangleCount;
    iVc1 triangleOffset;

    iVc1 unindexed;
    iVc1 loadOffset;
    iVc1 NB_mode;
    iVc1 clearDepth;

    // OctreeUniformStruct
    Vc4x4 project;
    Vc4x4 unproject;
    //Vc4x4 transform;
    //Vc4x4 transformInv;

    iVc1 maxDepth;
    iVc1 currentDepth;
    iVc1 nodeCount;
    iVc1 unk0;

    // MinmaxUniformStruct
    iVc1 heap;
    Vc1 prec;
    Vc1 _reserved0;
    Vc1 _reserved1;

    //AttributeUniformStruct attributeUniform;
    //GeometryUniformStruct geometryUniform;
    //OctreeUniformStruct octreeUniform;
    //MinmaxUniformStruct minmaxUniform;
};

RWStructuredBuffer<LightUniformStruct> lightNode : register(u10);
RWStructuredBuffer<RayBlockUniform> rayBlock : register(u11);
RWStructuredBuffer<GeometryBlockUniform> geometryBlock : register(u12);

#endif
