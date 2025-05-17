#include "web_client.h"

#include "launcher_settings.h"

WebClient::WebClient(WebAPI& webAPI, LauncherSettings& settings)
	: webAPI(webAPI)
	, settings(settings)
{
}

Future<bool> WebClient::updateProjectData(const String& url, const String& project, const String& username, const String& password)
{
	return getProjectData(url, project, username, password).then(aliveFlag, Executors::getMainUpdateThread(), [=](std::optional<WebProjectData> projectData)
	{
		if (projectData) {
			if (auto path = storeProjectData(url, project, *projectData)) {
				ConfigNode params;
				params["url"] = url;
				params["project"] = project;
				params["username"] = username;
				params["password"] = password;
				settings.addProject(*path);
				return true;
			}
		}
		return false;
	});
}

Future<std::optional<WebProjectData>> WebClient::getProjectData(const String& srcUrl, const String& project, const String& username, const String& password)
{
	auto url = srcUrl;
	if (url.endsWith("/")) {
		url = url.left(url.size() - 1);
	}

	Promise<std::optional<WebProjectData>> promise;
	auto result = promise.getFuture();

	login(url, project, username, password).then(aliveFlag, Executors::getCPU(), [=, promise = std::move(promise)](std::optional<String> token) mutable
	{
		if (token) {
			onAddFromURLLogin(url, project, *token, std::move(promise));
		} else {
			promise.setValue(std::nullopt);
		}
	});

	return result;
}

Future<std::optional<String>> WebClient::login(const String& baseURL, const String& project, const String& username, const String& password)
{
	const auto url = baseURL + "/sessions";

	ConfigNode reqInfo;
	reqInfo["username"] = username;
	reqInfo["password"] = password;
	reqInfo["project"] = project;

	auto request = webAPI.makeHTTPRequest(HTTPMethod::POST, url);
	request->setJsonBody(reqInfo);

	return request->send().then(aliveFlag, Executors::getImmediate(), [=] (std::unique_ptr<HTTPResponse> response) -> std::optional<String>
	{
		if (response->getResponseCode() == 0) {
			return {};
		} else if (response->getResponseCode() == 200) {
			const auto responseBody = JSONConvert::parseConfig(response->getBody());
			return responseBody["token"].asString("");
		} else {
			const auto responseBody = JSONConvert::parseConfig(response->getBody());
			Logger::logError("Error attempting to login: " + responseBody["errorMsg"].asString(""));
			return {};
		}
	});
}

void WebClient::onAddFromURLLogin(const String& baseURL, const String& project, const String& token, Promise<std::optional<WebProjectData>> promise)
{
	const auto url = baseURL + "/external-project-properties/" + Encode::encodeURL(project);
	auto request = webAPI.makeHTTPRequest(HTTPMethod::GET, url);
	request->setHeader("Authorization", "Bearer " + token);

	request->send().then(aliveFlag, Executors::getImmediate(), [promise = std::move(promise)] (std::unique_ptr<HTTPResponse> response) mutable
	{
		if (response->getResponseCode() == 200) {
			const auto responseBody = JSONConvert::parseConfig(response->getBody());
			promise.setValue(WebProjectData(responseBody));
		} else {
			promise.setValue(std::nullopt);
		}
	});
}

std::optional<Path> WebClient::storeProjectData(const String& url, const String& project, const WebProjectData& data)
{
	// TODO
	return {};
}

WebProjectData::WebProjectData(const ConfigNode& node)
{
	properties = Encode::decodeBase64(node["properties"].asString(""));
	icon = Encode::decodeBase64(node["icon"].asString(""));
}
