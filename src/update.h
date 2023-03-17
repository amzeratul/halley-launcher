#pragma once

#include <halley.hpp>

#include "new_version_info.h"

class Settings;

namespace Halley {
	class ILauncher;

	class Update : public UIWidget {
    public:
    	Update(UIFactory& factory, ILauncher& parent, NewVersionInfo info);
        ~Update() override;

        void onMakeUI() override;
        
    private:
    	UIFactory& factory;
        ILauncher& parent;
        NewVersionInfo info;
        Future<std::unique_ptr<HTTPResponse>> downloadFuture;

        void download();
        void onDownloadComplete(const Bytes& bytes);
        void onError(const String& error);
        void showMessage(const String& msg);
    };
}
