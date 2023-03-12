#pragma once

#include <halley.hpp>

#include "launcher_save_data.h"

namespace Halley {
	class ILauncher {
	public:
		virtual ~ILauncher() = default;
		virtual void switchTo(std::shared_ptr<UIWidget> widget) = 0;
		virtual void switchTo(const String& view) = 0;
	};

	class LauncherStage : public Stage, public ILauncher {
	public:
		LauncherStage();
		
		void init() override;

		void onVariableUpdate(Time) override;
		void onRender(RenderContext&) const override;

		void switchTo(std::shared_ptr<UIWidget> widget) override;
		void switchTo(const String& view) override;

	private:
		I18N i18n;
		std::unique_ptr<UIFactory> uiFactory;
		std::unique_ptr<UIRoot> ui;
		std::shared_ptr<UIWidget> topLevelUI;
		std::shared_ptr<UIWidget> curUI;

		Executor mainThreadExecutor;

		Sprite background;
		std::shared_ptr<LauncherSaveData> saveData;

		void makeSprites();
		void makeUI();
		void updateUI(Time time);
		void setCurrentUI(std::shared_ptr<UIWidget> ui);
	};
}
