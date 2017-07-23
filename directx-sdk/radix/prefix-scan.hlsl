
#include "./includes.hlsl"

groupshared uint local_sort[WG_COUNT];

uint prefix_sum(in uint data, inout uint total_sum) {
    local_sort[LC_IDX] = data;
    AllMemoryBarrierWithGroupSync();
    for (uint i = 1; i < WG_COUNT; i <<= 1) {
        uint tmp = local_sort[LC_IDX - i] * uint(LC_IDX >= i);
        AllMemoryBarrierWithGroupSync();
        local_sort[LC_IDX] += tmp;
        AllMemoryBarrierWithGroupSync();
    }
    total_sum = local_sort[WG_COUNT - 1];
    return local_sort[LC_IDX - 1] * uint(LC_IDX > 0);
}

groupshared uint seed;

[numthreads(1, WG_COUNT, 1)]
void CSMain( uint3 WorkGroupID : SV_GroupID, uint3 LocalInvocationID  : SV_GroupThreadID, uint3 GlobalInvocationID : SV_DispatchThreadID)
{
    WG_IDX = WorkGroupID.x;
    LC_IDX = LocalInvocationID.y;
    
#ifdef EMULATE_BALLOT
    LANE_IDX = LocalInvocationID.x;
#else
    LANE_IDX = WaveGetLaneIndex();
#endif

    if (LC_IDX == WG_COUNT - 1) seed = 0;
    AllMemoryBarrierWithGroupSync();
    for(uint d=0;d<RADICES;d++){
        uint val = 0;
        uint idx = d * WG_COUNT + LC_IDX;
        if (LC_IDX < WG_COUNT) val = Histogram[idx];
        uint total = 0;
        uint res = prefix_sum(val, total);
        if (LC_IDX < WG_COUNT) Histogram[idx] = res + seed;
        if (LC_IDX == WG_COUNT - 1) seed += res + val;
        AllMemoryBarrierWithGroupSync();
    }
}