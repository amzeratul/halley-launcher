#pragma once

#include <halley.hpp>
using namespace Halley;

class Settings {
public:
	bool isDirty() const;

	ConfigNode save() const;
	void load(const ConfigNode& node);

	void saveToFile(SystemAPI& system) const;
	void loadFromFile(SystemAPI& system);

	gsl::span<const String> getProjects() const;
	void setProjects(Vector<String> projects);
	bool addProject(String path);
	bool removeProject(const String& path);

private:
	mutable bool dirty = false;
	Vector<String> projects;
};
