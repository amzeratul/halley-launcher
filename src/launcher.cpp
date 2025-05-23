#include "launcher.h"

#include "launcher_stage.h"
#include "launcher_settings.h"

using namespace Halley;

void initOpenGLPlugin(IPluginRegistry &registry);
void initSDLSystemPlugin(IPluginRegistry &registry, std::optional<String> cryptKey);
void initSDLAudioPlugin(IPluginRegistry &registry);
void initSDLInputPlugin(IPluginRegistry &registry);
void initAsioPlugin(IPluginRegistry &registry);
void initDX11Plugin(IPluginRegistry &registry);
void initMetalPlugin(IPluginRegistry &registry);
void initHTTPLibPlugin(IPluginRegistry &registry);

HalleyLauncher::HalleyLauncher()
{}

HalleyLauncher::~HalleyLauncher()
{}

LauncherSettings& HalleyLauncher::getSettings()
{
	return *settings;
}

void HalleyLauncher::init(const Environment& environment, const Vector<String>& args)
{
	enum class ArgType {
		None,
		ProjectPath,
	};

	ArgType type = ArgType::None;
	projectPath = {};

	for (auto& arg : args) {
		if (arg.startsWith("--")) {
			if (arg == "--project") {
				type = ArgType::ProjectPath;
			}
		} else {
			if (type == ArgType::ProjectPath) {
				if (!projectPath) {
					projectPath = arg;
				} else {
					*projectPath += " " + arg;
				}
			}
		}
	}

}

int HalleyLauncher::initPlugins(IPluginRegistry& registry)
{
	initSDLSystemPlugin(registry, {});
	initAsioPlugin(registry);
	initSDLAudioPlugin(registry);
	initSDLInputPlugin(registry);
	initHTTPLibPlugin(registry);

#ifdef _WIN32
	initDX11Plugin(registry);
#elif __APPLE__
	initMetalPlugin(registry);
#else
	initOpenGLPlugin(registry);
#endif
	
	return HalleyAPIFlags::Video | HalleyAPIFlags::Audio | HalleyAPIFlags::Input | HalleyAPIFlags::Network | HalleyAPIFlags::Web;
}

ResourceOptions HalleyLauncher::initResourceLocator(const Path& gamePath, const Path& assetsPath, const Path& unpackedAssetsPath, ResourceLocator& locator)
{
	constexpr bool localAssets = false;
	if (localAssets) {
		locator.addFileSystem(unpackedAssetsPath);
	} else {
		const String packs[] = { "images.dat", "shaders.dat", "config.dat" };
		for (auto& pack: packs) {
			locator.addPack(Path(assetsPath) / pack);
		}
	}
	return {};
}

std::unique_ptr<Stage> HalleyLauncher::startGame()
{
	auto& api = getAPI();
	settings = std::make_unique<LauncherSettings>();
	settings->loadFromFile(*api.system);

	api.video->setWindow(WindowDefinition(WindowType::Window, Vector2i(800, 500), "Halley Launcher"));
	api.video->setVsync(true);

	return std::make_unique<LauncherStage>(projectPath);
}

String HalleyLauncher::getName() const
{
	return "Halley Launcher";
}

String HalleyLauncher::getDataPath(const Vector<String>& args) const
{
	return "halley/launcher";
}

bool HalleyLauncher::isDevMode() const
{
	return false;
}

bool HalleyLauncher::shouldCreateSeparateConsole() const
{
	return Debug::isDebug();
}

String HalleyLauncher::getDefaultColourScheme()
{
	return "colour_schemes/flat_cobalt_dark";
}

HalleyGame(HalleyLauncher);
