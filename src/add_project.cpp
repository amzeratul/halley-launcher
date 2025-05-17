#include "add_project.h"

#include "choose_project.h"
#include "launcher_project_properties.h"
#include "launcher_settings.h"
#include "launcher_stage.h"

AddProject::AddProject(UIFactory& factory, LauncherSettings& settings, ILauncher& parent)
	: UIWidget("add_project", {}, UISizer())
	, factory(factory)
	, settings(settings)
	, parent(parent)
{
	factory.loadUI(*this, "launcher/add_project");
}

void AddProject::onMakeUI()
{
	setHandle(UIEventType::ButtonClicked, "create", [=] (const UIEvent& event)
	{
		onNewProject();
	});

	setHandle(UIEventType::ButtonClicked, "addLocal", [=] (const UIEvent& event)
	{
		onAddFromDisk();
	});

	setHandle(UIEventType::ButtonClicked, "addURLStart", [=] (const UIEvent& event)
	{
		setPage(Page::OpenURL);
	});

	setHandle(UIEventType::ButtonClicked, "addFromURL", [=] (const UIEvent& event)
	{
		onAddFromURL();
	});

	setHandle(UIEventType::ButtonClicked, "cancel", [=] (const UIEvent& event)
	{
		if (page == Page::Main) {
			close();
		} else {
			cancel();
		}
	});

	setPage(Page::Main);
}

void AddProject::onNewProject()
{
	// TODO
}

void AddProject::onAddFromDisk()
{
	FileChooserParameters parameters;
	parameters.folderOnly = true;
	OS::get().openFileChooser(parameters).then(Executors::getMainUpdateThread(), [=] (std::optional<Path> path)
	{
		if (path) {
			addNewProject(ProjectLocation(path.value().getNativeString(false)));
		}
	});
}

void AddProject::addNewProject(const ProjectLocation& project)
{
	if (const auto properties = LauncherProjectProperties::getProjectProperties(project, &factory.getResources(), parent.getHalleyAPI().video)) {
		if (settings.addProject(project.path)) {
			close();
		} else {
			showError(LocalisedString::fromHardcodedString("Error: Project already exists."));
		}
	} else {
		showError(LocalisedString::fromHardcodedString("Error: Unable to add project."));
	}
}

void AddProject::showError(const String& errorMessage)
{
	return setError(LocalisedString::fromUserString(errorMessage));
}

void AddProject::showError(LocalisedString errorMessage)
{
	setError(std::move(errorMessage));
	setPage(Page::Main);
}

void AddProject::setError(LocalisedString msg)
{
	auto label = getWidgetAs<UILabel>("statusMessage");
	label->setActive(!msg.getString().isEmpty());
	label->setText(std::move(msg));
}

void AddProject::close()
{
	parent.switchTo(std::make_shared<ChooseProject>(factory, settings, parent));
}

void AddProject::cancel()
{
	setPage(Page::Main);
}

void AddProject::setPage(Page page)
{
	this->page = page;

	getWidget("mainPage")->setActive(page == Page::Main);
	getWidget("addURLPage")->setActive(page == Page::OpenURL);

	if (page == Page::OpenURL) {
		setURLPageEnabled(true);
	}
}

bool AddProject::canAddFromURL()
{
	auto url = getWidgetAs<UITextInput>("url");
	auto project = getWidgetAs<UITextInput>("project");
	auto username = getWidgetAs<UITextInput>("username");
	auto password = getWidgetAs<UITextInput>("password");

	return url->getText().startsWith("http") && !project->getText().isEmpty() && !username->getText().isEmpty() && !password->getText().isEmpty();
}

void AddProject::setURLPageEnabled(bool enabled)
{
	auto url = getWidget("url");
	auto project = getWidget("project");
	auto username = getWidget("username");
	auto password = getWidget("password");

	url->setEnabled(enabled);
	project->setEnabled(enabled);
	username->setEnabled(enabled);
	password->setEnabled(enabled);

	getWidget("addFromURL")->setEnabled(enabled && canAddFromURL());
}

void AddProject::onAddFromURL()
{
	if (!canAddFromURL()) {
		return;
	}
	setURLPageEnabled(false);

	auto url = getWidgetAs<UITextInput>("url")->getText();
	auto project = getWidgetAs<UITextInput>("project")->getText();
	auto username = getWidgetAs<UITextInput>("username")->getText();
	auto password = getWidgetAs<UITextInput>("password")->getText();

	parent.getWebClient().updateProjectData(url, project, username, password).then(aliveFlag, Executors::getMainUpdateThread(), [=](bool ok)
	{
		if (ok) {
			close();
		} else {
			showError("Unable to retrieve project data from web.");
			setURLPageEnabled(true);
		}
	});
}
