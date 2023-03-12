#pragma once

#include <halley.hpp>
using namespace Halley;

class ProjectProperties {
public:
    Path path;
    String name;
    Sprite icon;
    int halleyVerMajor = 0;
    int halleyVerMinor = 0;
    int halleyVerRevision = 0;

    static std::optional<ProjectProperties> getProjectProperties(const Path& path, Resources* resources = nullptr, VideoAPI* videoAPI = nullptr);
};
