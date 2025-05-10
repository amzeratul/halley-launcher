#pragma once

#include <halley.hpp>

#include "launcher_settings.h"

namespace Halley
{
	class HalleyLauncher final : public Game
	{
	public:
		HalleyLauncher();
		~HalleyLauncher();

		LauncherSettings& getSettings();

	protected:
		std::unique_ptr<LauncherSettings> settings;
		std::optional<String> projectPath;

		void init(const Environment& environment, const Vector<String>& args) override;
		int initPlugins(IPluginRegistry &registry) override;
		ResourceOptions initResourceLocator(const Path& gamePath, const Path& assetsPath, const Path& unpackedAssetsPath, ResourceLocator& locator) override;
		std::unique_ptr<Stage> startGame() override;

		String getName() const override;
		String getDataPath(const Vector<String>& args) const override;
		bool isDevMode() const override;
		bool shouldCreateSeparateConsole() const override;

		String getDefaultColourScheme() override;
	};
}
