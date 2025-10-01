#include "zp_cpp/gpu.hpp"

using namespace zp::gpu;

void zp::gpu::init_2(Instance* p_inst)
{
    descriptors::init(p_inst);
    passes::compute::init(p_inst);
    passes::graphics::init(p_inst);
}