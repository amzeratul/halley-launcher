#pragma once

#include <halley.hpp>

#include "launcher_settings.h"
using namespace Halley;

class LauncherProjectProperties {
public:
    Path path;
    String name;
    Sprite icon;
    HalleyVersion halleyVersion;
    HalleyVersion builtVersion;
    HalleyVersion cleanBuildIfOlderVersion;

    static std::optional<LauncherProjectProperties> getProjectProperties(const ProjectLocation& project, Resources* resources = nullptr, VideoAPI* videoAPI = nullptr);
};
