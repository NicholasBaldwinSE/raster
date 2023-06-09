#pragma once

// Local includes
#include "Polygon.hpp"
#include "Triangle.hpp"
#include "Surface.hpp"
#include "Sample.hpp"
#include "Light.hpp"

#include <vector>
#include <map>
#include <string>

struct World {
    std::vector<Polygon> polygons;
    std::vector<Triangle> triangles;
    // Name -> Surface map
    std::map<std::string, Surface> surfaces;
    glm::vec3 background;
    glm::vec3 eyep;
    glm::vec3 lookp;
    glm::vec3 up;
    glm::vec2 fov;
    glm::vec2 screen;
    Sample sample;
    std::vector<Light> lights;
};