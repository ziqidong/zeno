//
// Created by zhouhang on 2023/1/17.
//
#include <unordered_map>
#include <any>

#include <zeno/utils/disable_copy.h>
#include <zeno/utils/vec.h>
#include <memory>
#include "zeno/types/PrimitiveObject.h"

#ifndef ZENO_LIGHTCAMERAMANAGER_H
#define ZENO_LIGHTCAMERAMANAGER_H

namespace zenovis {

struct LightAreaData {
    bool enable;
    bool display;
    zeno::vec3f color;
    float intensity;
    zeno::vec3f translation;
    zeno::vec3f eulerXYZ;
    zeno::vec4f quatRotation;
    zeno::vec3f scaling;
};

struct LightCameraItem {
    std::unordered_map<std::string, std::any> channel_curves;
    float get_bool(std::string channel);
    float get_float(std::string channel);
    zeno::vec2f get_vec2f(std::string channel);
    zeno::vec3f get_vec3f(std::string channel);
    zeno::vec4f get_vec4f(std::string channel);
    zeno::vec4f get_quaternion(std::string channel);
    virtual std::shared_ptr<zeno::PrimitiveObject> proxy_prim() = 0;
};

struct LightAreaObject : LightCameraItem {
    LightAreaData get_data();
    std::shared_ptr<zeno::PrimitiveObject> proxy_prim() override;
};

struct LightCameraManager : zeno::disable_copy {
    std::unordered_map<std::string, std::shared_ptr<LightCameraItem>> items;
    void addLightArea();
    std::string next_id(const std::string& prefix);
};
}



#endif //ZENO_LIGHTCAMERAMANAGER_H
