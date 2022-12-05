#pragma once

#include "zxxglslvec.h"

struct MatOutput {
    vec3  basecolor;
    float metallic;
    float roughness;
    float subsurface;
    float specular;
    vec3  specularColor;
    float specularTint;
    float anisotropic;
    float sheen;
    float sheenTint;
    vec3 sheenTintColor;
    float clearcoat;
    float clearcoatGloss;
    float opacity;
    float ior;
    float flatness;
    float specTrans;
    float scatterDistance;
    float thin;
    float doubleSide;
    float scatterStep;
    float smoothness;
    vec3  sssColor;
    vec3  sssParam;
    float displacement;

    vec3 nrm;
    vec3 emission;
};

struct MatInput {
    vec3 pos;
    vec3 nrm;
    vec3 uv;
    vec3 clr;
    vec3 tang;
};
