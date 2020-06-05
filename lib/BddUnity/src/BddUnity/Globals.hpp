#pragma once

#include "Globals/Depth.hpp"
#include "Globals/MemPool.hpp"
#include "Globals/TestParams.hpp"

namespace BddUnity {
  namespace Globals {

    int snprintFlags(char * buffer, size_t size);
    int snprintMemory(char * buffer, size_t size);

  }
}