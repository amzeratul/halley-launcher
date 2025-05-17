#include "launcher_settings.h"

ProjectLocation::ProjectLocation(Path path, std::optional<String> url)
	: path(std::move(path))
	, url(std::move(url))
{
}

ProjectLocation::ProjectLocation(const ConfigNode& node)
{
	if (node.getType() == ConfigNodeType::String) {
		path = Path(node.asString());
	} else {
		path = node["path"].asString("");
		url = node["url"].asOptional<String>();
	}
}

ConfigNode ProjectLocation::toConfigNode() const
{
	ConfigNode::MapType result;
	result["path"] = path.toString();
	result["url"] = url;
	return result;
}

bool ProjectLocation::operator==(const Path& str) const
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

const ProjectLocation* LauncherSettings::tryGetProject(const Path& path) const
{
	for (const auto& project: projects) {
		if (project.path == path) {
			return &project;
		}
	}
	return nullptr;
}

bool LauncherSettings::addProject(Path path, std::optional<String> url)
{
	if (!std_ex::contains(projects, path)) {
		projects.insert(projects.begin(), path);
		dirty = true;
		return true;
	}
	return false;
}

bool LauncherSettings::removeProject(const Path& path)
{
	if (std_ex::contains(projects, path)) {
		std_ex::erase(projects, path);
		dirty = true;
		return true;
	}
	return false;
}

void LauncherSettings::bumpProject(const Path& path)
{
	const auto iter = std_ex::find(projects, path);
	if (iter != projects.end() && iter != projects.begin()) {
		auto project = std::move(*iter);
		projects.erase(iter);
		projects.insert(projects.begin(), std::move(project));
		dirty = true;
	}
}
