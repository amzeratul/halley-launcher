#include "launcher_stage.h"

#include "choose_project.h"
#include "launcher.h"
#include "launcher_save_data.h"
#include "launch_project.h"
#include "update.h"
using namespace Halley;

LauncherStage::LauncherStage(std::optional<String> initialProject)
	: initialProject(std::move(initialProject))
	, mainThreadExecutor(Executors::getMainUpdateThread())
{
}

void LauncherStage::init()
{
	saveData = std::make_shared<LauncherSaveData>(getSystemAPI().getStorageContainer(SaveDataType::SaveLocal));
	
	makeUI();
	makeSprites();

	newVersionCheck = getWebAPI().makeHTTPRequest(HTTPMethod::GET, "https://update.halley.io/halley-launcher.yaml")->send()
		.then([] (std::unique_ptr<HTTPResponse> response) -> NewVersionInfo
	{
		if (response->getResponseCode() == 200) {
			return NewVersionInfo::parse(response->getBody());
		} else {
			Logger::logError("Unable to retrieve new version info.");
			return {};
		}
	});
}

void LauncherStage::onVariableUpdate(Time time)
{
	mainThreadExecutor.runPending();

	if (newVersionCheck.isValid() && newVersionCheck.hasValue()) {
		newVersionInfo = newVersionCheck.get();
		newVersionCheck = {};
	}

	updateUI(time);

	auto& settings = dynamic_cast<HalleyLauncher&>(getGame()).getSettings();
	if (settings.isDirty()) {
		settings.saveToFile(getSystemAPI());
	}
}

void LauncherStage::onRender(RenderContext& context) const
{
	ui->render(context);

	context.bind([&](Painter& painter)
	{
		painter.clear(Colour4f()); // Needed for depth/stencil
		auto view = Rect4f(painter.getViewPort());

		// Background
		background.clone().setTexRect(view).setSize(view.getSize()).draw(painter);

		// UI
		SpritePainter spritePainter;
		spritePainter.startFrame(false);
		spritePainter.startRender(false, false, {});
		ui->draw(spritePainter, 1, 0);
		spritePainter.draw(1, painter);
	});
}

void LauncherStage::switchTo(std::shared_ptr<UIWidget> widget)
{
	setCurrentUI(widget);
}

void LauncherStage::switchTo(const String& view)
{
	if (view == "choose_project") {
		auto& settings = dynamic_cast<HalleyLauncher&>(getGame()).getSettings();
		setCurrentUI(std::make_shared<ChooseProject>(*uiFactory, settings, *this));
	} else if (view == "update") {
		setCurrentUI(std::make_shared<Update>(*uiFactory, *this, *newVersionInfo));
	} else {
		setCurrentUI({});
	}
}

const HalleyAPI& LauncherStage::getHalleyAPI() const
{
	return getAPI();
}

std::optional<NewVersionInfo> LauncherStage::getNewVersionInfo() const
{
	return newVersionInfo;
}

void LauncherStage::exit()
{
	getCoreAPI().quit(0);
}

void LauncherStage::makeSprites()
{
	auto mat = std::make_shared<Material>(getResource<MaterialDefinition>("Launcher/Background"));
	mat
		->set("u_col0", Colour4f(0.08f))
		.set("u_col1", Colour4f(0.07f))
		.set("u_distance", 6.0f);
	background = uiFactory->getColourScheme()->getBackground();
}

void LauncherStage::makeUI()
{
	uiFactory = getGame().createUIFactory(getAPI(), getResources(), i18n);

	ui = std::make_unique<UIRoot>(getAPI());
	ui->makeToolTip(uiFactory->getStyle("tooltip"));

	topLevelUI = uiFactory->makeUI("launcher/background");
	ui->addChild(topLevelUI);

	const auto bgCol = uiFactory->getColourScheme()->getColour("background0");
	getVideoAPI().getWindow().setTitleColour(bgCol, bgCol);

	if (initialProject) {
		auto& settings = dynamic_cast<HalleyLauncher&>(getGame()).getSettings();
		switchTo(std::make_shared<LaunchProject>(*uiFactory, settings, *this, *initialProject, false));
	} else {
		switchTo("choose_project");
	}
}

void LauncherStage::updateUI(Time time)
{
	const auto kb = getInputAPI().getKeyboard();
	const auto size = getVideoAPI().getWindow().getDefinition().getSize();
	const auto uiRect = Rect4f(Vector2f(), Vector2f(size)).grow(0, 0, 0, 0);

	topLevelUI->setMinSize(uiRect.getSize());
	topLevelUI->setPosition(uiRect.getTopLeft());
	ui->setRect(uiRect);
	ui->update(time, UIInputType::Mouse, getInputAPI().getMouse(), kb);
}

void LauncherStage::setCurrentUI(std::shared_ptr<UIWidget> ui)
{
	auto container = topLevelUI->getWidget("container");
	container->clear();
	if (curUI) {
		curUI->destroy();
	}
	if (ui) {
		container->add(ui, 1);
	}
	curUI = ui;
}
