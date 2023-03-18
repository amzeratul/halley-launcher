#pragma once

#include <halley.hpp>

#include "launcher_save_data.h"
#include "new_version_info.h"

namespace Halley {
	class ILauncher {
	public:
		virtual ~ILauncher() = default;
		virtual void switchTo(std::shared_ptr<UIWidget> widget) = 0;
		virtual void switchTo(const String& view) = 0;
		virtual const HalleyAPI& getHalleyAPI() const = 0;
		virtual std::optional<NewVersionInfo> getNewVersionInfo() const = 0;
		virtual void exit() = 0;
	};

	class LauncherStage : public Stage, public ILauncher {
	public:
		LauncherStage(std::optional<String> initialProject);
		
		void init() override;

		void onVariableUpdate(Time) override;
		void onRender(RenderContext&) const override;

		void switchTo(std::shared_ptr<UIWidget> widget) override;
		void switchTo(const String& view) override;
		const HalleyAPI& getHalleyAPI() const override;

		std::optional<NewVersionInfo> getNewVersionInfo() const override;
		void exit() override;

	private:
		std::optional<String> initialProject;
		I18N i18n;
		std::unique_ptr<UIFactory> uiFactory;
		std::unique_ptr<UIRoot> ui;
		std::shared_ptr<UIWidget> topLevelUI;
		std::shared_ptr<UIWidget> curUI;

		Executor mainThreadExecutor;

		Sprite background;
		std::shared_ptr<LauncherSaveData> saveData;

		Future<NewVersionInfo> newVersionCheck;
		std::optional<NewVersionInfo> newVersionInfo;

		void makeSprites();
		void makeUI();
		void updateUI(Time time);
		void setCurrentUI(std::shared_ptr<UIWidget> ui);
	};
}
