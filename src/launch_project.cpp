#include "launch_project.h"

#include "launcher_stage.h"
#include "project_properties.h"
using namespace Halley;

LaunchProject::LaunchProject(UIFactory& factory, Settings& settings, ILauncher& parent, Path path)
	: UIWidget("launch_project", Vector2f(), UISizer())
	, factory(factory)
	, settings(settings)
	, parent(parent)
	, path(std::move(path))
{
	const auto properties = ProjectProperties::getProjectProperties(this->path);
	if (!properties) {
		parent.switchTo("choose_project");
	} else if (properties->builtVersion != properties->halleyVersion) {
		factory.loadUI(*this, "launcher/launch_project");
		buildProject();
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
		parent.switchTo("choose_project");
	});
}

void LaunchProject::buildProject()
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
	const auto buildScript = path / "halley" / "scripts" / scriptName;
	const auto command = "\"" + buildScript.getNativeString() + "\" \"";

	runningCommand = OS::get().runCommandAsync(command, path.getNativeString(false), this);
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

	const auto dir = path / "halley" / "bin";
	const auto cmd = dir / "halley-editor.exe";

	if (Path::exists(cmd) && OS::get().runCommandDetached(cmd.getNativeString() + " " + path.getNativeString(false), dir.getNativeString(false))) {
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
