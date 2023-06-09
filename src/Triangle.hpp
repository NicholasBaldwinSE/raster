#pragma once

// Local includes.
#include "Surface.hpp"
#include "glm/glm.hpp"

struct Triangle {
    Surface surface;
    glm::vec3 verts[3];
    glm::vec3 norms[3];
};