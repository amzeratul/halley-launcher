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

void LaunchProject::update(Time t, bool moved)
{
	if (hasUI) {
		decltype(pendingProgress) prog;
		{
			auto lock = std::unique_lock(progressMutex);
			prog = pendingProgress;
		}

		getWidget("progress_bg")->setActive(prog->second > 0);
		if (prog->second > 0) {
			const auto size = getWidgetAs<UIImage>("progress_bg")->getSize();
			const float t = float(prog->first) / static_cast<float>(prog->second);
			getWidgetAs<UIImage>("progress")->getSprite().scaleTo(Vector2f::max(size * Vector2f(t, 1.0f), Vector2f(10.0f, 0.0f)));
		}
	}
}

void LaunchProject::loadUIIfNeeded()
{
	if (!hasUI) {
		factory.loadUI(*this, "launcher/launch_project");
	}
}

void LaunchProject::tryLaunching()
{
	setProgress(0, 0);

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

	setProgress(0, 1);
	parent.getWebClient().downloadEditor(version, [=, flag = NonOwningAliveFlag(aliveFlag)] (uint64_t cur, uint64_t total) -> bool
	{
		if (flag) {
			setProgress(cur, total);
			return true;
		} else {
			return false;
		}
	}).then(aliveFlag, Executors::getMainUpdateThread(), [=](Bytes bytes)
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
	getWidgetAs<UILabel>("status")->setText(LocalisedString::fromHardcodedString("Installing editor..."));
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

	const auto rootPath = projectPath;

	const auto n = zip.getNumFiles();
	uint64_t totalSize = 0;
	for (size_t i = 0; i < n; ++i) {
		totalSize += zip.getFileSize(i);
	}
	setProgress(0, totalSize);

	uint64_t totalExtracted = 0;
	for (size_t i = 0; i < n; ++i) {
		const auto name = zip.getFileName(i);
		auto fileBytes = zip.extractFile(i);
		if (!fileBytes.empty()) {
			const auto path = rootPath / name;
			const auto parentPath = path.parentPath().getString().cppStr();
			std::error_code ec;
			std::filesystem::create_directories(parentPath, ec);
			const bool ok = Path::writeFile(path, fileBytes);

			if (ok) {
				totalExtracted += zip.getFileSize(i);
				setProgress(totalExtracted, totalSize);
			} else {
				Concurrent::execute(Executors::getMainUpdateThread(), [this, name]()
				{
					log(LoggerLevel::Error, "Unable to extract file from zip: " + name);
				});
				return false;
			}
		}
	}

	return true;
}

void LaunchProject::launchProject()
{
	setProgress(0, 0);

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

void LaunchProject::setProgress(uint64_t progress, uint64_t total)
{
	auto lock = std::unique_lock(progressMutex);
	pendingProgress = { progress, total };
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
