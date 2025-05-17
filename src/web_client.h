#pragma once

#include <halley.hpp>
class LauncherSettings;
using namespace Halley;

struct WebProjectData {
	Bytes properties;
	Bytes icon;

	WebProjectData() = default;
	WebProjectData(const ConfigNode& node);
};

class WebClient {
public:
	WebClient(WebAPI& webAPI, LauncherSettings& settings);

	Future<bool> updateProjectData(const String& url, const String& project, const String& username, const String& password);

private:
	WebAPI& webAPI;
	LauncherSettings& settings;
	AliveFlag aliveFlag;

	Future<std::optional<WebProjectData>> getProjectData(const String& url, const String& project, const String& username, const String& password);
	Future<std::optional<String>> login(const String& url, const String& project, const String& username, const String& password);
	void onAddFromURLLogin(const String& url, const String& project, const String& token, Promise<std::optional<WebProjectData>> promise);
	std::optional<Path> storeProjectData(const String& url, const String& project, const WebProjectData& data);
};
