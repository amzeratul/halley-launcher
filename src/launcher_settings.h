#pragma once

#include <halley.hpp>
using namespace Halley;

class ProjectLocation {
public:
	Path path;
	std::optional<String> url;

	ProjectLocation() = default;
	ProjectLocation(Path path, std::optional<String> url = std::nullopt);
	ProjectLocation(const ConfigNode& node);

	ConfigNode toConfigNode() const;

	bool operator==(const Path& str) const;
};

class LauncherSettings {
public:
	bool isDirty() const;

	ConfigNode save() const;
	void load(const ConfigNode& node);

	void saveToFile(SystemAPI& system) const;
	void loadFromFile(SystemAPI& system);

	gsl::span<const ProjectLocation> getProjects() const;
	const ProjectLocation* tryGetProject(const Path& path) const;

	bool addProject(Path path, std::optional<String> url = std::nullopt);
	bool removeProject(const Path& path);
	void bumpProject(const Path& path);

private:
	mutable bool dirty = false;
	Vector<ProjectLocation> projects;
};
