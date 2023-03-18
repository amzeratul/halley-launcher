#pragma once

#include <halley.hpp>
using namespace Halley;

class ProjectProperties {
public:
    Path path;
    String name;
    Sprite icon;
    HalleyVersion halleyVersion;
    HalleyVersion builtVersion;
    HalleyVersion cleanBuildIfOlderVersion;

    static std::optional<ProjectProperties> getProjectProperties(const Path& path, Resources* resources = nullptr, VideoAPI* videoAPI = nullptr);
};
