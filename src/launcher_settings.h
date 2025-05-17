#pragma once

#include <halley.hpp>
using namespace Halley;

class ProjectLocation {
public:
	Path path;
	ConfigNode params;

	ProjectLocation() = default;
	ProjectLocation(Path path, ConfigNode params = {});
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

	bool addProject(Path path, ConfigNode params = {});
	bool addOrUpdateProject(Path path, ConfigNode params = {});
	bool removeProject(const Path& path);
	void bumpProject(const Path& path);

private:
	mutable bool dirty = false;
	Vector<ProjectLocation> projects;
};
