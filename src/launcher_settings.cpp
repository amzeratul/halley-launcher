#include "launcher_settings.h"

#include <filesystem>

ProjectLocation::ProjectLocation(Path path, ConfigNode params)
	: path(std::move(path))
	, params(std::move(params))
{
}

ProjectLocation::ProjectLocation(const ConfigNode& node)
{
	if (node.getType() == ConfigNodeType::String) {
		path = Path(node.asString());
	} else {
		path = node["path"].asString("");
		params = ConfigNode(node["params"]);
	}
}

ConfigNode ProjectLocation::toConfigNode() const
{
	ConfigNode::MapType result;
	result["path"] = path.toString();
	result["params"] = ConfigNode(params);
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

bool LauncherSettings::addProject(Path path, ConfigNode params)
{
	if (!std_ex::contains(projects, path)) {
		projects.insert(projects.begin(), ProjectLocation(std::move(path), std::move(params)));
		dirty = true;
		return true;
	}
	return false;
}

bool LauncherSettings::addOrUpdateProject(Path path, ConfigNode params)
{
	const auto iter = std_ex::find(projects, path);
	if (iter != projects.end()) {
		iter->params = std::move(params);
		return true;
	} else {
		return addProject(std::move(path), std::move(params));
	}
}

bool LauncherSettings::removeProject(const Path& path)
{
	const auto iter = std_ex::find(projects, path);
	if (iter != projects.end()) {
		if (iter->params.hasKey("url")) {
			std::error_code ec;
			std::filesystem::remove_all(iter->path.getString().cppStr(), ec);
			if (ec) {
				return false;
			}
		}

		projects.erase(iter);
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
