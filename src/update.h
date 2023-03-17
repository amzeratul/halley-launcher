#pragma once

#include <halley.hpp>

#include "new_version_info.h"

class Settings;

namespace Halley {
	class ILauncher;

	class Update : public UIWidget {
    public:
    	Update(UIFactory& factory, ILauncher& parent, NewVersionInfo info);

        void onMakeUI() override;
        
    private:
    	UIFactory& factory;
        ILauncher& parent;
        NewVersionInfo info;
    };
}
