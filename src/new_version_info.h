#pragma once

#include <halley.hpp>
using namespace Halley;

class NewVersionInfo {
public:
    constexpr static int currentVersion = 1;
    
    int version;
    String downloadURL;
    Bytes signature;

    bool isNewVersion() const;

    static NewVersionInfo parse(const Bytes& bytes);
};
