//
// Created by zhouhang on 2023/1/17.
//
#include <zeno/types/UserData.h>
#include "zenovis/LightCameraManager.h"
#include "zeno/logger.h"

void zenovis::LightCameraManager::addLightArea() {
    auto nid = next_id("LightArea");
    auto item = std::make_shared<LightAreaObject>();
    items[nid] = item;
}

std::string zenovis::LightCameraManager::next_id(const std::string& prefix) {
    std::string nid {};
    int i = 0;
    while (true) {
        nid = zeno::format("{}-{}", prefix, i);
        if (!items.count(nid)) {
            break;
        }
        i++;
    }
    return nid;
}

std::shared_ptr<zeno::PrimitiveObject> zenovis::LightAreaObject::proxy_prim() {
    auto prim = std::make_shared<zeno::PrimitiveObject>();
    auto& user_data = prim->userData();
    user_data.set2("_translate", zeno::vec3f(0, 0, 0));
    user_data.set2("_rotate", zeno::vec4f(0, 0, 0, 1));
    user_data.set2("_scale", zeno::vec3f(1, 1, 1));
    user_data.set2<int>("interactive", 1);
    prim->verts.resize(4);
    prim->verts[0] = zeno::vec3f(0.5, 0, 0.5);
    prim->verts[1] = zeno::vec3f(0.5, 0, -0.5);
    prim->verts[2] = zeno::vec3f(-0.5, 0, 0.5);
    prim->verts[3] = zeno::vec3f(-0.5, 0, -0.5);
    prim->tris.resize(2);
    prim->tris[0] = zeno::vec3i(0, 3, 1);
    prim->tris[1] = zeno::vec3i(2, 3, 0);
    return prim;
}
