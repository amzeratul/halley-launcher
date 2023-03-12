#include "settings.h"

bool Settings::isDirty() const
{
	return dirty;
}

ConfigNode Settings::save() const
{
	ConfigNode::MapType result;
	result["projects"] = projects;
	return result;
}

void Settings::load(const ConfigNode& node)
{
	projects = node["projects"].asVector<String>({});
	dirty = false;
}

void Settings::saveToFile(SystemAPI& system) const
{
	ConfigFile file;
	file.getRoot() = save();
	system.getStorageContainer(SaveDataType::SaveLocal)->setData("settings", Serializer::toBytes(file));
	dirty = false;
}

void Settings::loadFromFile(SystemAPI& system)
{
	auto data = system.getStorageContainer(SaveDataType::SaveLocal)->getData("settings");
	if (!data.empty()) {
		auto config = Deserializer::fromBytes<ConfigFile>(data);
		load(config.getRoot());
	}
}

gsl::span<const String> Settings::getProjects() const
{
	return projects;
}

void Settings::setProjects(Vector<String> projects)
{
	if (this->projects != projects) {
		this->projects = std::move(projects);
		dirty = true;
	}
}

bool Settings::addProject(String path)
{
	if (!std_ex::contains(projects, path)) {
		projects.push_back(path);
		dirty = true;
		return true;
	}
	return false;
}

bool Settings::removeProject(const String& path)
{
	if (std_ex::contains(projects, path)) {
		std_ex::erase(projects, path);
		dirty = true;
		return true;
	}
	return false;
}
