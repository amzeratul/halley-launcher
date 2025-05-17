#include "web_client.h"

#include <filesystem>

#include "launcher_settings.h"

WebClient::WebClient(WebAPI& webAPI, LauncherSettings& settings, Path projectsFolder)
	: webAPI(webAPI)
	, settings(settings)
	, projectsFolder(std::move(projectsFolder))
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
				settings.addProject(*path, std::move(params));
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
	Hash::Hasher hasher;
	hasher.feed(url);
	const auto hash = hasher.digest();

	String projectId = project + "-" + toString(hash, 16);
	Path basePath = projectsFolder / projectId;

	for (const auto& file: data.files) {
		const auto path = basePath / file.first;
		//OS::get().createDirectories(path.parentPath());
		std::error_code ec;
		std::filesystem::create_directories(path.parentPath().getString().cppStr(), ec);
		bool ok = Path::writeFile(path, file.second);
		if (!ok) {
			Logger::logError("Could not write " + toString(file.second.size()) + " bytes to " + path.getNativeString(false));
			return std::nullopt;
		}
	}

	return basePath;
}

WebProjectData::WebProjectData(const ConfigNode& node)
{
	if (node.hasKey("files")) {
		for (const auto& fileNode: node["files"].asSequence()) {
			const auto path = fileNode["path"].asString("");
			const auto bytes = Encode::decodeBase64(fileNode["bytes"].asString(""));
			if (!path.isEmpty() && !bytes.empty() && !path.contains("..")) {
				files.emplace_back(path, bytes);
			}
		}
	}
}

Future<Bytes> WebClient::downloadEditor(HalleyVersion version, std::function<bool(uint64_t, uint64_t)> callback)
{
	auto request = webAPI.makeHTTPRequest(HTTPMethod::GET, "https://update.halley.io/halley-editor-bins/halley-editor-" + version.toString() + ".zip");

	if (callback) {
		request->setProgressCallback(std::move(callback));
	}

	return request->send().then([] (std::unique_ptr<HTTPResponse> response) -> Bytes
	{
		if (response->getResponseCode() == 200) {
			return response->getBody();
		} else {
			return {};
		}
	});
}
