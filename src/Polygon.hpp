#pragma once

// Local includes.
#include "Surface.hpp"
#include "glm/glm.hpp"

struct Polygon {
    Surface surface;
    glm::vec3 verts[4];
};