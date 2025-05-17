#include "launch_project.h"

#include "launcher_stage.h"
#include "launcher_project_properties.h"
using namespace Halley;

LaunchProject::LaunchProject(UIFactory& factory, LauncherSettings& settings, ILauncher& parent, ProjectLocation project, bool safeMode)
	: UIWidget("launch_project", Vector2f(), UISizer())
	, factory(factory)
	, settings(settings)
	, parent(parent)
	, project(std::move(project))
	, safeMode(safeMode)
{
	const auto properties = LauncherProjectProperties::getProjectProperties(this->project);
	if (!properties) {
		parent.switchTo("choose_project");
	} else if (properties->builtVersion != properties->halleyVersion) {
		factory.loadUI(*this, "launcher/launch_project");
		buildProject(properties->builtVersion < properties->cleanBuildIfOlderVersion);
	} else {
		launchProject();
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

void LaunchProject::buildProject(bool clean)
{
	getWidgetAs<UILabel>("status")->setText(LocalisedString::fromHardcodedString("Building..."));

	const String scriptName = [] ()
	{
		if constexpr (getPlatform() == GamePlatform::Windows) {
			return "build_editor.bat";
		} else {
			throw Exception("No project build script available for this platform.", HalleyExceptions::Tools);
		}
	}();
	const auto buildScript = Path(project.path) / "halley" / "scripts" / scriptName;
	const auto command = "\"" + buildScript.getNativeString() + "\"" + (clean ? " --clean" : "");

	runningCommand = OS::get().runCommandAsync(command, project.path.getNativeString(false), this);
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

void LaunchProject::launchProject()
{
	if (hasUI) {
		getWidgetAs<UILabel>("status")->setText(LocalisedString::fromHardcodedString("Launching..."));
	}

	const auto dir = Path(project.path) / "halley" / "bin";
	const auto cmd = dir / "halley-editor.exe";
	const auto params = "--project \"" + project.path
		+ "\" --launcher \"" + (parent.getHalleyAPI().core->getEnvironment().getProgramPath() / "halley-launcher.exe").getNativeString() + "\""
		+ (safeMode ? " --dont-load-dll" : "");

	if (Path::exists(cmd) && OS::get().runCommandDetached(cmd.getNativeString() + " " + params, dir.getNativeString(false))) {
		parent.getHalleyAPI().core->quit(0);
	} else {
		if (!hasUI) {
			factory.loadUI(*this, "launcher/launch_project");
			getWidgetAs<UILabel>("status")->setText(LocalisedString::fromHardcodedString("Launching..."));
		}
		log(LoggerLevel::Error, "Editor not found at " + cmd.getNativeString());
	}
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
