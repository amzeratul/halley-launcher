#pragma once

#include <halley.hpp>
using namespace Halley;

class NewVersionInfo {
public:
    constexpr static int currentVersion = 0;
    
    int version;
    String download;

    bool isNewVersion() const;

    static NewVersionInfo parse(const Bytes& bytes);
};
