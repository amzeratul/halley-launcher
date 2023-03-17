#include "update.h"

#include "launcher_stage.h"

Update::Update(UIFactory& factory, ILauncher& parent, NewVersionInfo info)
	: UIWidget("update", {}, UISizer())
	, factory(factory)
	, parent(parent)
	, info(std::move(info))
{
	factory.loadUI(*this, "launcher/update");

	download();
}

Update::~Update()
{
	if (downloadFuture.isValid()) {
		downloadFuture.cancel();
	}
}

void Update::onMakeUI()
{
	setHandle(UIEventType::ButtonClicked, "cancel", [=] (const UIEvent& event)
	{
		parent.switchTo("choose_project");
	});
}

void Update::download()
{
	auto request = parent.getHalleyAPI().web->makeHTTPRequest(HTTPMethod::GET, info.download);
	downloadFuture = request->send();
	downloadFuture.then([this](std::unique_ptr<HTTPResponse> response)
	{
		if (response->getResponseCode() == 200) {
			onDownloadComplete(response->getBody());
		} else {
			onError("HTTP Error " + toString(response->getResponseCode()) + ": " + info.download);
		}
	});
}

void Update::onDownloadComplete(const Bytes& bytes)
{
	showMessage("Extracting...");
	// TODO: extract zip
	// TODO: run self-update
}

void Update::onError(const String& error)
{
	showMessage(error);
	getWidget("progress_bg")->setActive(false);
}

void Update::showMessage(const String& msg)
{
	getWidgetAs<UILabel>("status")->setText(LocalisedString::fromUserString(msg));
}
