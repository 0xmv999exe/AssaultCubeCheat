#pragma once
#include "game.h"

namespace Features {
    void RunESP(Entity* local, Entity** eList, int count, float* matrix);
    void RunAimbot(Entity* local, Entity** eList, int count, float* matrix);
    void RunMisc(Entity* local);
}
