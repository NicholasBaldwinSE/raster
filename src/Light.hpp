#pragma once

// Local includes.
#include "glm/glm.hpp"

// We don't actually use any of these other than point,
// but I figured I'd define them while I was here.
static enum LightTypes {
    lightPoint,
    lightSpot,
    lightDirectional,
    lightExtended,
    lightArea,
} lights;

struct Light {
    LightTypes type;
    float intensity;
    glm::vec3 pos;
};