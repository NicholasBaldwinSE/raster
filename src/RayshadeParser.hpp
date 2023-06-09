#pragma once

// Local includes
#include "World.hpp"

// Global includes.
#include <fstream>
#include <iostream>
#include <string>

// Define keywords.
static enum Keywords {
    keywordBackground,
    keywordEyep,
    keywordLookp,
    keywordUp,
    keywordFov,
    keywordScreen,
    keywordSample,
    keywordLight,
    keywordSurface, // Surface is special, and will need its own sub-parser.
    keywordPolygon,
    keywordTriangle
} keywords;

static std::map<std::string, Keywords> keywordMap {
    {"background", keywordBackground},
    {"eyep", keywordEyep},
    {"lookp", keywordLookp},
    {"up", keywordUp},
    {"fov", keywordFov},
    {"screen", keywordScreen},
    {"sample", keywordSample},
    {"light", keywordLight},
    {"surface", keywordSurface},
    {"polygon", keywordPolygon},
    {"triangle", keywordTriangle}
};

World* parseFile(std::string rayFilePath) {
    // Create new world object.
    World* world = new World;

    // Open rayshade file for parsing.
    std::fstream rayFile(rayFilePath);

    // Assert that the file is actually opened.
    if (!rayFile.is_open()) {
        return NULL;
    }

    // Read in first keyword
    std::string keyword;
    rayFile >> keyword;

    // Exit early if parsing fails on first keyword. 
    // This should cover empty files.
    if (keyword.empty()) {
        return NULL;
    }

    bool skipFlag = false;

    // Loop while there are still keywords.
    while (!keyword.empty() && !rayFile.eof()) {
        // Check which keyword it is.
        // Switch trick used found from: https://www.codeguru.com/cplusplus/switch-on-strings-in-c/
        switch (keywordMap[keyword]) {
            case keywordBackground: {
                float x, y, z;

                rayFile >> x >> y >> z;
                world->background = {x, y, z};

            } break;
            case keywordEyep: {
                float x, y, z;

                rayFile >> x >> y >> z;
                world->eyep = {x, y, z};

            } break;
            case keywordLookp: {
                float x, y, z;

                rayFile >> x >> y >> z;
                world->lookp = {x, y, z};

            } break;
            case keywordUp: {
                float x, y, z;

                rayFile >> x >> y >> z;
                world->up = {x, y, z};

            } break;
            case keywordFov: {
                int w, h;

                rayFile >> w >> h;
                world->fov = {w, h};

            } break;
            case keywordScreen: {
                int w, h;

                rayFile >> w >> h;
                world->screen = {w, h};

            } break;
            case keywordSample: {
                Sample sample;
                int samples;
                std::string nojitter;

                // We're going to always assume nojitter. Why? 
                // Because parsing sucks.
                rayFile >> samples >> nojitter;

                sample.samples = samples;

                if (nojitter.compare("nojitter") == 0) {
                    sample.nojitter = true;
                }

                world->sample = sample;

            } break;
            case keywordLight: {
                // Lights technically don't require a type, but for simplicity of parsing
                // we're going to assume they always have one.
                Light light;
                float intensity, x, y, z;
                std::string type;

                rayFile >> intensity >> type >> x >> y >> z;

                light.intensity = intensity;
                light.pos = {x, y, z};

                // We're only going to check for point lights.
                // We have the ability to care about other ones, 
                // but they also require extra parsing, something I don't care
                // to do if we're not given files that use it.
                if (type.compare("point") == 0) {
                    light.type = LightTypes::lightPoint;
                }

                world->lights.push_back(light);

            } break;
            case keywordSurface: {
                // Subparser done differently, mostly for my sanity
                Surface surface;
                std::string surfaceName;
                // We set defaults since these might not even exist.
                // If we defined refraction here, which we don't, it'd be 1.
                glm::vec3 ambient, diffuse, specular;
                float specpow, reflect;

                ambient = diffuse = specular = { 0, 0, 0 };
                specpow = reflect = 0;

                rayFile >> surfaceName;

                std::string surfaceParam;
                rayFile >> surfaceParam;
                
                while (true) {
                    if (surfaceParam.compare("ambient") == 0) {
                        float x, y, z;
                        rayFile >> x >> y >> z;
                        ambient = { x, y, z };
                    }
                    else if (surfaceParam.compare("diffuse") == 0) {
                        float x, y, z;
                        rayFile >> x >> y >> z;
                        diffuse = { x, y, z };
                    }
                    else if (surfaceParam.compare("specular") == 0) {
                        float x, y, z;
                        rayFile >> x >> y >> z;
                        specular = { x, y, z };
                    }
                    else if (surfaceParam.compare("specpow") == 0) {
                        float x;
                        rayFile >> x;
                        specpow = x;
                    }
                    else if (surfaceParam.compare("reflect") == 0) {
                        float x;
                        rayFile >> x;
                        reflect = x;
                    }
                    else {
                        break;
                    }

                    rayFile >> surfaceParam;
                }

                surface.ambient = ambient;
                surface.diffuse = diffuse;
                surface.specular = specular;
                surface.specpow = specpow;
                surface.reflect = reflect;
                surface.name = surfaceName;

                world->surfaces.emplace(std::make_pair(surfaceName, surface));
                keyword = surfaceParam;
                skipFlag = true;
            } break;
            case keywordPolygon: {
                Polygon polygon;
                std::string surfaceName;
                float w1, w2, w3
                    , x1, x2, x3
                    , y1, y2, y3
                    , z1, z2, z3;

                rayFile >> surfaceName 
                    >> w1 >> w2 >> w3 
                    >> x1 >> x2 >> x3 
                    >> y1 >> y2 >> y3 
                    >> z1 >> z2 >> z3;

                polygon.surface = world->surfaces[surfaceName];
                polygon.verts[0] = {w1, w2, w3};
                polygon.verts[1] = {x1, x2, x3};
                polygon.verts[2] = {y1, y2, y3};
                polygon.verts[3] = {z1, z2, z3};

                world->polygons.push_back(polygon);

            } break;
            case keywordTriangle: {
                Triangle triangle;
                std::string surfaceName;
                float x1, x2, x3
                    , y1, y2, y3
                    , z1, z2, z3
                    , xn1, xn2, xn3
                    , yn1, yn2, yn3
                    , zn1, zn2, zn3;

                rayFile >> surfaceName 
                    >> x1 >> x2 >> x3
                    >> xn1 >> xn2 >> xn3
                    >> y1 >> y2 >> y3
                    >> yn1 >> yn2 >> yn3
                    >> z1 >> z2 >> z3
                    >> zn1 >> zn2 >> zn3;
                
                triangle.surface = world->surfaces[surfaceName];
                triangle.verts[0] = {x1, x2, x3};
                triangle.verts[1] = {y1, y2, y3};
                triangle.verts[2] = {z1, z2, z3};
                triangle.norms[0] = {xn1, xn2, xn3};
                triangle.norms[1] = {yn1, yn2, yn3};
                triangle.norms[2] = {zn1, zn2, zn3};

                world->triangles.push_back(triangle);
            } break;
            default: {
                
            } break;
        }

        // We use this to more precisely capture keywords, due to
        // the subparser.
        if (!skipFlag) {
            // Reads next keyword.
            rayFile >> keyword;
        }
        skipFlag = false;
    }

    rayFile.close();

    return world;
}