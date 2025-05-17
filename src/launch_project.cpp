#include "launch_project.h"

#include <filesystem>

#include "launcher_stage.h"
#include "launcher_project_properties.h"
using namespace Halley;

LaunchProject::LaunchProject(UIFactory& factory, LauncherSettings& settings, ILauncher& parent, ProjectLocation project, bool safeMode)
	: UIWidget("launch_project", Vector2f(), UISizer())
	, factory(factory)
	, settings(settings)
	, parent(parent)
	, projectLocation(std::move(project))
	, safeMode(safeMode)
{
	if (this->projectLocation.params.hasKey("url")) {
		checkForProjectUpdates();
	} else {
		tryLaunching();
	}
}

void LaunchProject::onMakeUI()
{
	hasUI = true;

	setHandle(UIEventType::ButtonClicked, "cancel", [=] (const UIEvent& event)
	{
		if (runningCommand.isValid()) {
			runningCommand.cancel();
			runningCommand = {};
		}
		Concurrent::execute(Executors::getMainUpdateThread(), [=]() {
			parent.switchTo("choose_project");
		});
	});
}

void LaunchProject::loadUIIfNeeded()
{
	if (!hasUI) {
		factory.loadUI(*this, "launcher/launch_project");
	}
}

void LaunchProject::tryLaunching()
{
	const auto properties = LauncherProjectProperties::getProjectProperties(projectLocation);
	if (!properties) {
		parent.switchTo("choose_project");
	} else if (properties->builtVersion != properties->halleyVersion) {
		if (projectLocation.params.hasKey("url")) {
			downloadEditor(properties->halleyVersion);
		} else {
			buildProject(properties->builtVersion < properties->cleanBuildIfOlderVersion);
		}
	} else {
		launchProject();
	}
}

void LaunchProject::buildProject(bool clean)
{
	loadUIIfNeeded();

	getWidgetAs<UILabel>("status")->setText(LocalisedString::fromHardcodedString("Building..."));

	const String scriptName = [] ()
	{
		if constexpr (getPlatform() == GamePlatform::Windows) {
			return "build_editor.bat";
		} else {
			throw Exception("No project build script available for this platform.", HalleyExceptions::Tools);
		}
	}();
	const auto buildScript = Path(projectLocation.path) / "halley" / "scripts" / scriptName;
	const auto command = "\"" + buildScript.getNativeString() + "\"" + (clean ? " --clean" : "");

	runningCommand = OS::get().runCommandAsync(command, projectLocation.path.getNativeString(false), this);
	runningCommand.then(Executors::getMainUpdateThread(), [=] (int returnValue)
	{
		if (returnValue == 0) {
			// Success
			log(LoggerLevel::Info, "Build successful.");
			launchProject();
		} else {
			// Fail
			log(LoggerLevel::Error, "Build failed with error code " + toString(returnValue));
		}
	});
}

void LaunchProject::downloadEditor(HalleyVersion version)
{
	loadUIIfNeeded();
	getWidgetAs<UILabel>("status")->setText(LocalisedString::fromHardcodedString("Downloading editor..."));

	parent.getWebClient().downloadEditor(version).then(aliveFlag, Executors::getMainUpdateThread(), [=](Bytes bytes)
	{
		if (bytes.empty()) {
			log(LoggerLevel::Error, "Unable to download Halley Editor version " + version.toString());
		} else {
			log(LoggerLevel::Info, "Download successful");
			installEditor(bytes);
		}
	});
}

void LaunchProject::installEditor(Bytes data)
{
	Concurrent::execute([this, data = std::move(data), path = projectLocation.path] () mutable -> bool
	{
		return doInstallEditor(std::move(data), path);
	}).then(aliveFlag, Executors::getMainUpdateThread(), [=] (bool ok)
	{
		if (ok) {
			launchProject();
		} else {
			log(LoggerLevel::Error, "Unable to extract Halley Editor - make sure it's not already running.");
		}
	});
}

bool LaunchProject::doInstallEditor(Bytes bytes, const Path& projectPath)
{
	ZipFile zip;
	bool success = zip.open(std::move(bytes));
	if (!success) {
		Concurrent::execute(Executors::getMainUpdateThread(), [this]()
		{
			log(LoggerLevel::Error, "Unable to parse zip file.");
		});
		return false;
	}

	const auto rootPath = projectPath / "halley";

	const auto n = zip.getNumFiles();
	for (size_t i = 0; i < n; ++i) {
		const auto name = zip.getFileName(i);
		auto fileBytes = zip.extractFile(i);
		if (!fileBytes.empty()) {
			const auto path = rootPath / name;
			const auto parentPath = path.parentPath().getString().cppStr();
			std::error_code ec;
			std::filesystem::create_directories(parentPath, ec);
			const bool ok = Path::writeFile(path, fileBytes);

			if (!ok) {
				return false;
			}
		}
	}

	return true;
}

void LaunchProject::launchProject()
{
	if (hasUI) {
		getWidgetAs<UILabel>("status")->setText(LocalisedString::fromHardcodedString("Launching..."));
	}

	const auto dir = Path(projectLocation.path) / "halley" / "bin";
	const auto cmd = dir / "halley-editor.exe";
	const auto params = "--project \"" + projectLocation.path
		+ "\" --launcher \"" + (parent.getHalleyAPI().core->getEnvironment().getProgramPath() / "halley-launcher.exe").getNativeString() + "\""
		+ (safeMode ? " --dont-load-dll" : "");

	if (Path::exists(cmd) && OS::get().runCommandDetached(cmd.getNativeString() + " " + params, dir.getNativeString(false))) {
		parent.getHalleyAPI().core->quit(0);
	} else {
		loadUIIfNeeded();
		getWidgetAs<UILabel>("status")->setText(LocalisedString::fromHardcodedString("Launching..."));
		log(LoggerLevel::Error, "Editor not found at " + cmd.getNativeString());
	}
}

void LaunchProject::checkForProjectUpdates()
{
	loadUIIfNeeded();
	getWidgetAs<UILabel>("status")->setText(LocalisedString::fromHardcodedString("Checking for updates..."));

	const auto url = projectLocation.params["url"].asString("");
	const auto project = projectLocation.params["project"].asString("");
	const auto username = projectLocation.params["username"].asString("");
	const auto password = projectLocation.params["password"].asString("");
	parent.getWebClient().updateProjectData(url, project, username, password).then(aliveFlag, Executors::getMainUpdateThread(), [=] (bool ok)
	{
		if (!ok) {
			log(LoggerLevel::Warning, "Failed to update project.");
		}
		tryLaunching();
	});
}

void LaunchProject::log(LoggerLevel level, std::string_view msg)
{
	const char* styleNames[] = { "ui_logDevText", "ui_logInfoText", "ui_logWarningText", "ui_logErrorText" };

	auto label = std::make_shared<UILabel>("", factory.getStyle("labelLight"), LocalisedString::fromUserString(msg));
	label->setColour(factory.getColourScheme()->getColour(styleNames[static_cast<int>(level)]));

	const auto logList = getWidget("log");
	logList->add(label);
	logList->layout();
	logList->sendEvent(UIEvent(UIEventType::MakeAreaVisible, logList->getId(), label->getRect() - logList->getPosition()));

	Logger::log(level, msg);
}
