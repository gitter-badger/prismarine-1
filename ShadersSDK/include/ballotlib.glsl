// ordered increment
#if (!defined(FRAGMENT_SHADER) && !defined(ORDERING_NOT_REQUIRED))

// ballot math (alpha version)
#define WARP_SIZE 32
#ifdef EMULATE_BALLOT

#define   LC_IDX (gl_LocalInvocationID.x / WARP_SIZE)
#define LANE_IDX (gl_LocalInvocationID.x % WARP_SIZE)
#define UVEC_BALLOT_WARP uint

shared uint ballotCache[WORK_SIZE];
shared uint invocationCache[WORK_SIZE/WARP_SIZE][WARP_SIZE];

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
#define LANE_IDX (gl_LocalInvocationID.x % gl_SubGroupSizeARB)
#define UVEC_BALLOT_WARP uvec2

uvec2 genLtMask(){
    //return unpackUint2x32(gl_SubGroupLtMaskARB);

    uvec2 mask = uvec2(0, 0);
    if (LANE_IDX >= 64) {
        mask = uvec2(0xFFFFFFFF, 0xFFFFFFFF);
    } else 
    if (LANE_IDX >= 32) {
        mask = uvec2(0xFFFFFFFF, (1 << (LANE_IDX-32))-1);
    } else {
        mask = uvec2((1 << LANE_IDX)-1, 0);
    }
    return mask;
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





// bit logic
const int bx = 1, by = 2, bz = 4, bw = 8;
const int bswiz[8] = {1, 2, 4, 8, 16, 32, 64, 128};

int cB(in bool a, in int swizzle){
    return a ? swizzle : 0;
}


int cB4(in bvec4 a){
    ivec4 mx4 = mix(ivec4(0), ivec4(bx, by, bz, bw), a);
    ivec2 mx2 = mx4.xy | mx4.wz; // swizzle or
    return (mx2.x | mx2.y); // rest of
}

int cB2(in bvec2 a){
    ivec2 mx = mix(ivec2(0), ivec2(bx, by), a);
    return (mx.x | mx.y);
}

bvec2 cI2(in int a){
    return bvec2(
        (a & bx) > 0, 
        (a & by) > 0
    );
}

bool anyB(in int a){
    return a > 0;
}

bool allB2(in int a){
    return a == 3;
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
