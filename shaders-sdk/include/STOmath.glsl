#ifndef _STOMATH_H
#define _STOMATH_H


// roundly comparsion functions
bool lessEqualF(in float a, in float b) { return (b-a) > -PZERO; }
bool lessF(in float a, in float b) { return (b-a) >= PZERO; }
bool greaterEqualF(in float a, in float b) { return (a-b) > -PZERO; }
bool greaterF(in float a, in float b) { return (a-b) >= PZERO; }
bool equalF(in float a, in float b) { return abs(a-b) < PZERO; }


// vector math utils
float sqlen(in vec3 a) { return dot(a, a); }
float sqlen(in vec2 a) { return dot(a, a); }
float sqlen(in float v) { return v * v; }
float mlength(in vec3 mcolor){ return max(mcolor.x, max(mcolor.y, mcolor.z)); }
vec4 divW(in vec4 aw){ return aw / aw.w; }


// unorm compasion
bool iseq8(float a, float cmp){ return abs(fma(a, 255.f, -cmp)) < 0.00001f; }
bool iseq16(float a, float cmp){ return abs(fma(a, 65535.f, -cmp)) < 0.00001f; }


// memory managment
void swap(inout int a, inout int b){ int t = a; a = b; b = t; }
uint exchange(inout uint mem, in uint v){ uint tmp = mem; mem = v; return tmp; }
int exchange(inout int mem, in int v){ int tmp = mem; mem = v; return tmp; }


// logical functions
bvec2 not2(in bvec2 a) { return bvec2(!a.x, !a.y); }
bvec2 and2(in bvec2 a, in bvec2 b) { return bvec2(a.x && b.x, a.y && b.y); }
bvec2 or2(in bvec2 a, in bvec2 b) { return bvec2(a.x || b.x, a.y || b.y); }


// mixing functions
void mixed(inout vec3 src, inout vec3 dst, in float coef){ dst *= coef; src *= 1.0f - coef; }
void mixed(inout vec3 src, inout vec3 dst, in vec3 coef){ dst *= coef; src *= 1.0f - coef; }


// matrix math
vec4 mult4(in vec4 vec, in mat4 mat){
    return vec4(dot(mat[0], vec), dot(mat[1], vec), dot(mat[2], vec), dot(mat[3], vec));
}

vec4 mult4(in mat4 tmat, in vec4 vec){
    return fma(tmat[0], vec.xxxx, fma(tmat[1], vec.yyyy, fma(tmat[2], vec.zzzz, tmat[3] * vec.wwww)));
}


int modi(in int a, in int b){
    return (a % b + b) % b;
}

uint btc(in uint vlc){
    return vlc == 0 ? 0 : bitCount(vlc);
}


// ordered increment
#if (!defined(FRAGMENT_SHADER) && !defined(ORDERING_NOT_REQUIRED))

// ballot math (alpha version)
#define WARP_SIZE 32
#ifdef EMULATE_BALLOT

#define   LC_IDX (gl_LocalInvocationID.x / WARP_SIZE)
#define LANE_IDX (gl_LocalInvocationID.x % WARP_SIZE)
#define UVEC_BALLOT_WARP uint

shared uint ballotCache[WORK_SIZE];
shared uint invocationCache[WORK_SIZE][WARP_SIZE];

uint genLtMask(){
    return (1 << LANE_IDX)-1;
}

uint ballotHW(in bool val) {
    ballotCache[LC_IDX] = 0;
    // warp can be have barrier, but is not required
    atomicOr(ballotCache[LC_IDX], uint(val) << LANE_IDX);
    // warp can be have barrier, but is not required
    return ballotCache[LC_IDX];
}

uint bitCount64(in uint a64) {
    return btc(a64);
}


float readLane(in float val, in int lane) {
    // warp can be have barrier, but is not required
    atomicExchange(invocationCache[LC_IDX][LANE_IDX], floatBitsToUint(val));
    // warp can be have barrier, but is not required
    return uintBitsToFloat(invocationCache[LC_IDX][lane]);
}

uint readLane(in uint val, in int lane) {
    // warp can be have barrier, but is not required
    atomicExchange(invocationCache[LC_IDX][LANE_IDX], val);
    // warp can be have barrier, but is not required
    return invocationCache[LC_IDX][lane];
}

int readLane(in int val, in int lane) {
    // warp can be have barrier, but is not required
    atomicExchange(invocationCache[LC_IDX][LANE_IDX], uint(val));
    // warp can be have barrier, but is not required
    return int(invocationCache[LC_IDX][lane]);
}

int firstActive(){
    return findLSB(ballotHW(true).x);
}

#else

#define   LC_IDX (gl_LocalInvocationID.x / gl_SubGroupSizeARB)
#define LANE_IDX (gl_SubGroupInvocationARB)
#define UVEC_BALLOT_WARP uvec2

uvec2 genLtMask(){
    return unpackUint2x32(gl_SubGroupLtMaskARB);
}

uint bitCount64(in uvec2 lh) {
    return btc(lh.x) + btc(lh.y);
}


vec4 readLane(in vec4 val, in int lane){
    return readInvocationARB(val, lane);
}

float readLane(in float val, in int lane){
    return readInvocationARB(val, lane);
}

uint readLane(in uint val, in int lane){
    return readInvocationARB(val, lane);
}

int readLane(in int val, in int lane){
    return readInvocationARB(val, lane);
}

uvec2 ballotHW(in bool val) {
    return unpackUint2x32(ballotARB(val)) & uvec2(
        gl_SubGroupSizeARB >= 32 ? 0xFFFFFFFF : ((1 << gl_SubGroupSizeARB)-1), 
        gl_SubGroupSizeARB >= 64 ? 0xFFFFFFFF : ((1 << (gl_SubGroupSizeARB-32))-1)
    );
}

int firstActive(){
    UVEC_BALLOT_WARP bits = ballotHW(true);
    int lv = findLSB(bits.x);
    return (lv >= 0 ? lv : (32+findLSB(bits.y)));
}



#endif

#define initAtomicIncFunction(mem, fname, T)\
T fname(in bool value){ \
    int activeLane = firstActive();\
    UVEC_BALLOT_WARP bits = ballotHW(value);\
    T sumInOrder = T(bitCount64(bits));\
    T idxInOrder = T(bitCount64(genLtMask() & bits));\
    return readLane(LANE_IDX == activeLane ? atomicAdd(mem,  mix(0, sumInOrder, LANE_IDX == activeLane)) : 0, activeLane) + idxInOrder; \
}

#define initAtomicDecFunction(mem, fname, T)\
T fname(in bool value){ \
    int activeLane = firstActive();\
    UVEC_BALLOT_WARP bits = ballotHW(value);\
    T sumInOrder = T(bitCount64(bits));\
    T idxInOrder = T(bitCount64(genLtMask() & bits));\
    return readLane(LANE_IDX == activeLane ? atomicAdd(mem, -mix(0, sumInOrder, LANE_IDX == activeLane)) : 0, activeLane) - idxInOrder; \
}

#else

#define initAtomicIncFunction(mem, fname, T)\
T fname(in bool value){ \
    return atomicAdd(mem, T(value)); \
}

#define initAtomicDecFunction(mem, fname, T)\
T fname(in bool value){ \
    return atomicAdd(mem, -T(value)); \
}

#endif





#if defined(ENABLE_AMD_INSTRUCTION_SET) || defined(ENABLE_NVIDIA_INSTRUCTION_SET)
#define INDEX16 uint16_t
#define M16(m, i) (m[i])
#else
#define INDEX16 uint
#define M16(m, i) (bitfieldExtract(m[i/2], int(16*(i%2)), 16))
#endif

#ifdef ENABLE_INT16_LOADING
#define INDICE_T INDEX16
#define PICK(m, i) M16(m, i)
#else
#define INDICE_T uint
#define PICK(m, i) m[i]
#endif


#endif