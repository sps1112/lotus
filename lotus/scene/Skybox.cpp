#include "lotus/scene.h"

namespace Lotus
{
    Skybox::Skybox(const std::vector<std::string>& paths, const Resource::SRefModel& model_,
                   const Rendering::SRefShader& shader_)
    {
        cubemap = std::make_shared<Lotus::Resource::Cubemap>(paths);
        model.model = model_;
        model.shader = shader_;
    }
}
