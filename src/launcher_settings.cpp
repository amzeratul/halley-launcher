#include "launcher_settings.h"

ProjectLocation::ProjectLocation(String path, std::optional<String> url)
	: path(std::move(path))
	, url(std::move(url))
{
}

ProjectLocation::ProjectLocation(const ConfigNode& node)
{
	if (node.getType() == ConfigNodeType::String) {
		path = node.asString();
	} else {
		path = node["path"].asString("");
		url = node["url"].asOptional<String>();
	}
}

ConfigNode ProjectLocation::toConfigNode() const
{
	ConfigNode::MapType result;
	result["path"] = path;
	result["url"] = url;
	return result;
}

bool ProjectLocation::operator==(const String& str) const
{
	return path == str;
}

bool LauncherSettings::isDirty() const
{
	return dirty;
}

ConfigNode LauncherSettings::save() const
{
	ConfigNode::MapType result;
	result["projects"] = projects;
	return result;
}

void LauncherSettings::load(const ConfigNode& node)
{
	projects = node["projects"].asVector<ProjectLocation>({});
	dirty = false;
}

void LauncherSettings::saveToFile(SystemAPI& system) const
{
	ConfigFile file;
	file.getRoot() = save();
	system.getStorageContainer(SaveDataType::SaveLocal)->setData("settings", Serializer::toBytes(file));
	dirty = false;
}

void LauncherSettings::loadFromFile(SystemAPI& system)
{
	auto data = system.getStorageContainer(SaveDataType::SaveLocal)->getData("settings");
	if (!data.empty()) {
		auto config = Deserializer::fromBytes<ConfigFile>(data);
		load(config.getRoot());
	}
}

gsl::span<const ProjectLocation> LauncherSettings::getProjects() const
{
	return projects;
}

const ProjectLocation* LauncherSettings::tryGetProject(const String& path) const
{
	for (const auto& project: projects) {
		if (project.path == path) {
			return &project;
		}
	}
	return nullptr;
}

bool LauncherSettings::addProject(String path, std::optional<String> url)
{
	if (!std_ex::contains(projects, path)) {
		projects.insert(projects.begin(), path);
		dirty = true;
		return true;
	}
	return false;
}

bool LauncherSettings::removeProject(const String& path)
{
	if (std_ex::contains(projects, path)) {
		std_ex::erase(projects, path);
		dirty = true;
		return true;
	}
	return false;
}

void LauncherSettings::bumpProject(const String& path)
{
	removeProject(path);
	addProject(path);
}
