#include "update.h"

#include <filesystem>

#include "launcher_stage.h"

Update::Update(UIFactory& factory, ILauncher& parent, NewVersionInfo info)
	: UIWidget("update", {}, UISizer())
	, factory(factory)
	, parent(parent)
	, info(std::move(info))
{
	factory.loadUI(*this, "launcher/update");
}

Update::~Update()
{
	if (downloadFuture.isValid()) {
		downloadFuture.cancel();
	}
	if (extractFuture.isValid()) {
		extractFuture.cancel();
	}
}

void Update::onMakeUI()
{
	setHandle(UIEventType::ButtonClicked, "cancel", [=] (const UIEvent& event)
	{
		parent.switchTo("choose_project");
	});
}

void Update::onAddedToRoot(UIRoot& root)
{
	download(info.download);
}

void Update::update(Time t, bool moved)
{
	doUpdateProgress();
}

void Update::download(const String& url, int depth)
{
	downloading = true;
	auto request = parent.getHalleyAPI().web->makeHTTPRequest(HTTPMethod::GET, url);
	auto weakThis = weak_from_this();
	updateDownloadProgress(0, 1);
	request->setProgressCallback([weakThis] (uint64_t cur, uint64_t total) -> bool
	{
		auto ptr = weakThis.lock();
		if (ptr) {
			Concurrent::execute(Executors::getMainUpdateThread(), [=] ()
			{
				std::dynamic_pointer_cast<Update>(ptr)->updateDownloadProgress(cur, total);
			});
		}
		return !!ptr;
	});
	downloadFuture = request->send();
	downloadFuture.then(Executors::getMainUpdateThread(), [this, depth](std::unique_ptr<HTTPResponse> response)
	{
		onDownloadComplete(response->getResponseCode(), response->moveBody(), response->getRedirectLocation(), depth);
	});
}

void Update::onDownloadComplete(int responseCode, Bytes bytes, String redirect, int depth)
{
	downloading = false;
	if (responseCode == 301 && depth < 3) {
		download(redirect, depth + 1);
		return;
	} else if (responseCode != 200) {
		downloading = false;
		onError("HTTP Error " + toString(responseCode) + ": " + info.download);
		return;
	}

	latestProgress = {};
	showMessage("Checking file...");

	extractFuture = Concurrent::execute([=, bytes = std::move(bytes)]() mutable
	{
		if (isValidSignature(bytes)) {
			extract(std::move(bytes));
		} else {
			Concurrent::execute(Executors::getMainUpdateThread(), [=]()
			{
				showMessage("Invalid signature, unable to auto-update.");
			});
		}
	});
}

void Update::extract(Bytes bytes)
{
	Concurrent::execute(Executors::getMainUpdateThread(), [=] ()
	{
		showMessage("Extracting...");
	});

	ZipFile zip;
	bool success = zip.open(std::move(bytes));
	if (!success) {
		onError("Invalid file.");
		return;
	}

	size_t totalSize = 0;
	const auto n = zip.getNumFiles();
	for (size_t i = 0; i < n; ++i) {
		totalSize += zip.getFileSize(i);
	}

	const auto rootPath = parent.getHalleyAPI().core->getEnvironment().getProgramPath() / ".." / "tmp";

	size_t runningTotal = 0;
	for (size_t i = 0; i < n; ++i) {
		const auto name = zip.getFileName(i);
		auto fileBytes = zip.extractFile(i);
		if (fileBytes.empty()) {
			continue;
		}

		updateExtractProgress(runningTotal + fileBytes.size() / 2, totalSize);

		const auto path = rootPath / name;
		const auto parentPath = path.parentPath().getString().cppStr();
		std::error_code ec;
		std::filesystem::create_directories(parentPath, ec);
		Path::writeFile(path, fileBytes);

		runningTotal += fileBytes.size();
		updateExtractProgress(runningTotal, totalSize);
	}

	runUpdate();
}

void Update::runUpdate()
{
	const auto cwd = parent.getHalleyAPI().core->getEnvironment().getProgramPath() / ".." / "tmp" / "scripts";
	const auto path = cwd / "update.bat";
	OS::get().runCommandDetached(path.getNativeString(false), cwd.getNativeString(false));
	parent.exit();
}

void Update::onError(const String& error)
{
	showMessage(error);
	latestProgress = std::nullopt;
	getWidget("progress_bg")->setActive(false);
}

void Update::showMessage(const String& msg)
{
	getWidgetAs<UILabel>("status")->setText(LocalisedString::fromUserString(msg));
}

void Update::updateDownloadProgress(uint64_t cur, uint64_t total)
{
	if (downloading) {
		latestProgress = { cur, total };
	}
}

void Update::updateExtractProgress(uint64_t cur, uint64_t total)
{
	latestProgress = { cur, total };
}

void Update::doUpdateProgress()
{
	if (latestProgress) {
		const auto size = getWidgetAs<UIImage>("progress_bg")->getSize();
		const float t = float(latestProgress->first) / static_cast<float>(latestProgress->second);
		getWidgetAs<UIImage>("progress")->getSprite().scaleTo(Vector2f::max(size * Vector2f(t, 1.0f), Vector2f(10.0f, 0.0f)));

		if (latestProgress->second > 1) {
			const String msg = downloading ? "Downloading... " : "Extracting... ";
			showMessage(msg + String::prettySize(latestProgress->first) + " / " + String::prettySize(latestProgress->second));
		}

		latestProgress = {};
	}
}

bool Update::isValidSignature(const Bytes& bytes)
{
	// TODO: needs cryptography support
	return true;
}
