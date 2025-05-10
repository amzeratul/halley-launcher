#pragma once

#include <halley.hpp>
using namespace Halley;

class ProjectLocation {
public:
	String path;
	std::optional<String> url;

	ProjectLocation() = default;
	ProjectLocation(String path, std::optional<String> url = std::nullopt);
	ProjectLocation(const ConfigNode& node);

	ConfigNode toConfigNode() const;

	bool operator==(const String& str) const;
};

class LauncherSettings {
public:
	bool isDirty() const;

	ConfigNode save() const;
	void load(const ConfigNode& node);

	void saveToFile(SystemAPI& system) const;
	void loadFromFile(SystemAPI& system);

	gsl::span<const ProjectLocation> getProjects() const;
	const ProjectLocation* tryGetProject(const String& path) const;

	bool addProject(String path, std::optional<String> url = std::nullopt);
	bool removeProject(const String& path);
	void bumpProject(const String& path);

private:
	mutable bool dirty = false;
	Vector<ProjectLocation> projects;
};
