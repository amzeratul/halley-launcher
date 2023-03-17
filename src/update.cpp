#include "update.h"

#include "launcher_stage.h"

Update::Update(UIFactory& factory, ILauncher& parent, NewVersionInfo info)
	: UIWidget("update", {}, UISizer())
	, factory(factory)
	, parent(parent)
	, info(std::move(info))
{
	factory.loadUI(*this, "launcher/update");
}

void Update::onMakeUI()
{
	setHandle(UIEventType::ButtonClicked, "cancel", [=] (const UIEvent& event)
	{
		parent.switchTo("choose_project");
	});
}
