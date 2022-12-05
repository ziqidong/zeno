#include <zeno/zeno.h>
#include <zeno/extra/ShaderNode.h>
#include <zeno/types/StringObject.h>
#include <zeno/types/ShaderObject.h>
#include <zeno/types/MaterialObject.h>
#include <zeno/types/ListObject.h>
#include <zeno/types/TextureObject.h>
#include <zeno/utils/string.h>

namespace zeno {


struct ShaderFinalize : INode {
    virtual void apply() override {
        EmissionPass em;
        auto backend = get_param<std::string>("backend");
        if (backend == "HLSL")
            em.backend = em.HLSL;
        else if (backend == "GLSL")
            em.backend = em.GLSL;

        if (has_input("commonCode"))
            em.commonCode += get_input<StringObject>("commonCode")->get();

        auto code = em.finalizeCode({
            {1, "mat_base"},
            {3, "mat_basecolor"},
            {1, "mat_metallic"},
            {1, "mat_roughness"},
            {1, "mat_specular"},
            {1, "mat_subsurface"},
            {1, "mat_thickness"},
            {3, "mat_sssParam"},
            {3, "mat_sssColor"},
            {1, "mat_sssScale"},
            {1, "mat_foliage"},
            {1, "mat_skin"},
            {1, "mat_curvature"},
            {3, "mat_specularColor"},
            {1, "mat_specularTint"},
            {1, "mat_anisotropic"},
            {1, "mat_sheen"},
            {1, "mat_sheenTint"},
            {3, "mat_sheenTintColor"},
            {1, "mat_clearcoat"},
            {1, "mat_clearcoatRoughness"},
            {1, "mat_specTrans"},
            {1, "mat_ior"},
            {1, "mat_flatness"},
            {1, "mat_scatterDistance"},
            {1, "mat_scatterStep"},
            {1, "mat_thin"},
            {1, "mat_doubleSide"},
            {3, "mat_normal"},
            {1, "mat_displacement"},
            {1, "mat_smoothness"},
            {3, "mat_emission"},
            {1, "mat_emissionIntensity"},
            {1, "mat_zenxposure"},
            {1, "mat_ao"},
            {1, "mat_toon"},
            {1, "mat_stroke"},
            {3, "mat_shape"},
            {1, "mat_style"},
            {1, "mat_strokeNoise"},
            {3,"mat_shad"},
            {3,"mat_strokeTint"},
            {1,"mat_opacity"},
            {1,"mat_reflection"},
            {1,"mat_reflectID"},
            {1,"mat_isCamera"},
            {1,"mat_isVoxelDomain"}
        }, {
            get_input<IObject>("base", std::make_shared<NumericObject>(1.0f)),
            get_input<IObject>("baseColor", std::make_shared<NumericObject>(vec3f(1.0f))),
            get_input<IObject>("metallic", std::make_shared<NumericObject>(float(0.0f))),
            get_input<IObject>("roughness", std::make_shared<NumericObject>(float(0.4f))),
            get_input<IObject>("specular", std::make_shared<NumericObject>(float(0.5f))),
            get_input<IObject>("subsurface", std::make_shared<NumericObject>(float(0.0f))),
            get_input<IObject>("thickness", std::make_shared<NumericObject>(float(0.0f))),
            get_input<IObject>("sssRadius", std::make_shared<NumericObject>(vec3f(1.0f))),
            get_input<IObject>("sssColor", std::make_shared<NumericObject>(vec3f(1.0f))),
            get_input<IObject>("sssScale", std::make_shared<NumericObject>(float(1.0f))),
            get_input<IObject>("foliage", std::make_shared<NumericObject>(float(0.0f))),
            get_input<IObject>("skin", std::make_shared<NumericObject>(float(0.0f))),
            get_input<IObject>("curvature", std::make_shared<NumericObject>(float(0.0f))),
            get_input<IObject>("specularColor", std::make_shared<NumericObject>(vec3f(1))),
            get_input<IObject>("specularTint", std::make_shared<NumericObject>(float(0.0f))),
            get_input<IObject>("anisotropic", std::make_shared<NumericObject>(float(0.0f))),
            get_input<IObject>("sheen", std::make_shared<NumericObject>(float(0.0f))),
            get_input<IObject>("sheenTint", std::make_shared<NumericObject>(float(0.5f))),
            get_input<IObject>("sheenColor", std::make_shared<NumericObject>(vec3f(1,1,1))),
            get_input<IObject>("clearcoat", std::make_shared<NumericObject>(float(0.0f))),
            get_input<IObject>("clearcoatRoughness", std::make_shared<NumericObject>(float(1.0f))),
            get_input<IObject>("transmission", std::make_shared<NumericObject>(float(0.0f))),
            get_input<IObject>("specularIOR", std::make_shared<NumericObject>(float(1.5f))),
            get_input<IObject>("flatness", std::make_shared<NumericObject>(float(0.0f))),
            get_input<IObject>("transmissionDistance", std::make_shared<NumericObject>(float(10000))),
            get_input<IObject>("transmissionStep", std::make_shared<NumericObject>(float(0))),
            get_input<IObject>("thinWalled", std::make_shared<NumericObject>(float(0.0f))),
            get_input<IObject>("doubleSide", std::make_shared<NumericObject>(float(0.0f))),
            get_input<IObject>("normal", std::make_shared<NumericObject>(vec3f(0, 0, 1))),
            get_input<IObject>("displacement", std::make_shared<NumericObject>(float(0.0f))),
            get_input<IObject>("smoothness", std::make_shared<NumericObject>(float(1.0f))),
            get_input<IObject>("emissionColor", std::make_shared<NumericObject>(vec3f(1))),
            get_input<IObject>("emission", std::make_shared<NumericObject>(float(0))),
            get_input<IObject>("exposure", std::make_shared<NumericObject>(float(1.0f))),
            get_input<IObject>("ao", std::make_shared<NumericObject>(float(1.0f))),
            get_input<IObject>("toon", std::make_shared<NumericObject>(float(0.0f))),
            get_input<IObject>("stroke", std::make_shared<NumericObject>(float(1.0f))),
            get_input<IObject>("shape", std::make_shared<NumericObject>(vec3f(-0.5,0.5,0))),
            get_input<IObject>("style", std::make_shared<NumericObject>(float(1.0))),
            get_input<IObject>("strokeNoise", std::make_shared<NumericObject>(float(1))),
            get_input<IObject>("shad", std::make_shared<NumericObject>(vec3f(0,0,0))),
            get_input<IObject>("strokeTint", std::make_shared<NumericObject>(vec3f(0,0,0))),
            get_input<IObject>("opacity", std::make_shared<NumericObject>(float(1.0))),
            get_input<IObject>("reflection", std::make_shared<NumericObject>(float(0.0))),
            get_input<IObject>("reflectID", std::make_shared<NumericObject>(float(-1))),
            get_input<IObject>("isCamera", std::make_shared<NumericObject>(float(0))),
            get_input<IObject>("isVoxelDomain", std::make_shared<NumericObject>(float(0))),

            
        });
        auto commonCode = em.getCommonCode();

        auto mtl = std::make_shared<MaterialObject>();
        mtl->frag = std::move(code);
        mtl->common = std::move(commonCode);
        if (has_input("extensionsCode"))
            mtl->extensions = get_input<zeno::StringObject>("extensionsCode")->get();

        if (has_input("tex2dList"))
        {
            auto tex2dList = get_input<ListObject>("tex2dList")->get<zeno::Texture2DObject>();
            for (const auto &tex: tex2dList)
            {
                auto texId = mtl->tex2Ds.size();
                auto texCode = "uniform sampler2D zenotex" + std::to_string(texId) + ";\n";
			    mtl->tex2Ds.push_back(tex);
                mtl->common.insert(0, texCode);
            }
        }

        //if (has_input("mtlid"))
        //{
            mtl->mtlidkey = get_input2<std::string>("mtlid");
        //}

        set_output("mtl", std::move(mtl));
    }
};

ZENDEFNODE(ShaderFinalize, {
    {
        {"string", "mtlid", "Mat1"},
        {"float", "base", "1"},
        {"vec3f", "baseColor", "1,1,1"},
        {"float", "metallic", "0.0"},
        {"float", "roughness", "0.4"},
        {"float", "specular", "0.5"},
        {"float", "specularIOR", "1.5"},
        {"vec3f", "specularColor", "1,1,1"},
        {"float", "specularTint", "0.0"},
        {"float", "anisotropic", "0.0"},
        {"float", "transmission", "0.0"},
        {"float", "transmissionDistance", "10000"},
        {"float", "subsurface", "0.0"},
        {"vec3f", "sssColor", "1.0,1.0,1.0"},
        {"vec3f", "sssRadius", "1,1,1"},
        {"float", "sssScale", "1"},
        {"float", "sheen", "0.0"},
        {"float", "sheenTint", "0.0"},
        {"vec3f", "sheenColor", "1,1,1"},
        {"float", "clearcoat", "0.0"},
        {"float", "clearcoatRoughness", "0.0"},
        {"float", "emission", "0"},
        {"vec3f", "emissionColor", "1,1,1"},
        {"float", "opacity", "1"},
        {"float", "thinWalled", "0.0"},
        {"float", "flatness", "0.0"},
        {"float", "doubleSide", "0.0"},
        {"float", "smoothnesss", "1.0"},
        {"vec3f", "normal", "0,0,1"},
        {"float", "displacement", "0"},
        {"string", "commonCode"},
        {"string", "extensionsCode"},
        {"list", "tex2dList"},//TODO: bate's asset manager
    },
    {
        {"MaterialObject", "mtl"},
    },
    {
        {"enum GLSL HLSL", "backend", "GLSL"},
    },
    {"shader"},
});


}
