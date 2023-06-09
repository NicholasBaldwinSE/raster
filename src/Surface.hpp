#pragma once

// Local includes.
#include "glm/glm.hpp"

// Global includes
#include <string>

struct Surface {
    std::string name;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float specpow;
    float reflect;
};