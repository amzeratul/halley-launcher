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
        void onAddedToRoot(UIRoot& root) override;

        void update(Time t, bool moved) override;

    private:
    	UIFactory& factory;
        ILauncher& parent;
        NewVersionInfo info;

        bool downloading = false;
		Future<std::unique_ptr<HTTPResponse>> downloadFuture;
        Future<void> extractFuture;

        std::optional<std::pair<uint64_t, uint64_t>> latestProgress;

        void download(const String& url, int depth = 0);
        void onDownloadComplete(int responseCode, Bytes bytes, String redirect, int depth);

        void extract(Bytes bytes);

        void runUpdate();

        void onError(const String& error);
        void showMessage(const String& msg);

        void updateDownloadProgress(uint64_t cur, uint64_t total);
        void updateExtractProgress(uint64_t cur, uint64_t total);
        void doUpdateProgress();

        bool isValidSignature(const Bytes& bytes);
    };
}
