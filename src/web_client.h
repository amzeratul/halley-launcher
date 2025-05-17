#pragma once

#include <halley.hpp>
class LauncherSettings;
using namespace Halley;

struct WebProjectData {
	Vector<std::pair<String, Bytes>> files;

	WebProjectData() = default;
	WebProjectData(const ConfigNode& node);
};

class WebClient {
public:
	WebClient(WebAPI& webAPI, LauncherSettings& settings, Path projectsFolder);

	Future<bool> updateProjectData(const String& url, const String& project, const String& username, const String& password);
	Future<Bytes> downloadEditor(HalleyVersion version, std::function<bool(uint64_t, uint64_t)> progressCallback = {});

private:
	WebAPI& webAPI;
	LauncherSettings& settings;
	Path projectsFolder;
	AliveFlag aliveFlag;

	Future<std::optional<WebProjectData>> getProjectData(const String& url, const String& project, const String& username, const String& password);
	Future<std::optional<String>> login(const String& url, const String& project, const String& username, const String& password);
	void onAddFromURLLogin(const String& url, const String& project, const String& token, Promise<std::optional<WebProjectData>> promise);
	std::optional<Path> storeProjectData(const String& url, const String& project, const WebProjectData& data);
};
