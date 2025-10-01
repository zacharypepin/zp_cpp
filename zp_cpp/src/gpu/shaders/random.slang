module random;

uint NextRandom(inout uint rngState) {
    rngState = rngState * 747796405u + 2891336453u;
    uint result = ((rngState >> ((rngState >> 28) + 4)) ^ rngState) * 277803737u;
    result = (result >> 22) ^ result;
    return result;
}

float RandomValue(inout uint rngState) {
    return float(NextRandom(rngState)) / 4294967295.0; 
}

float RandomValueNormalDistribution(inout uint rngState) {
    float theta = 2.0 * 3.1415926 * RandomValue(rngState);
    float rho = sqrt(-2.0 * log(RandomValue(rngState)));
    return rho * cos(theta);
}

public float3 RandomDirection(inout uint rngState) {
    float x = RandomValueNormalDistribution(rngState);
    float y = RandomValueNormalDistribution(rngState);
    float z = RandomValueNormalDistribution(rngState);
    return normalize(float3(x, y, z));
}