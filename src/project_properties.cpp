#include "project_properties.h"

std::optional<ProjectProperties> ProjectProperties::getProjectProperties(const Path& path, Resources* resources, VideoAPI* videoAPI)
{
	const auto propertiesBytes = Path::readFile(path / "halley_project" / "properties.yaml");
	if (propertiesBytes.empty()) {
		return {};
	}

	ProjectProperties result;

	if (resources && videoAPI) {
		const auto iconBytes = Path::readFile(path / "halley_project" / "icon48.png");
		if (!iconBytes.empty()) {
			auto image = std::make_unique<Image>(iconBytes.byte_span());
			result.icon.setImage(*resources, *videoAPI, std::move(image));
		}
	}

	const auto config = YAMLConvert::parseConfig(propertiesBytes);
	result.name = config.getRoot()["name"].asString("Unknown");
	result.path = path;

	const auto halleyVersionBytes = Path::readFileLines(path / "halley" / "include" / "halley_version.hpp");
	if (!halleyVersionBytes.empty()) {
		for (auto& line: halleyVersionBytes) {
			if (line.startsWith("#define")) { 
				int value = 0;
				for (const auto& v: line.split(' ')) {
					if (v.isInteger()) {
						value = v.toInteger();
						break;
					}
				}
				if (line.contains("HALLEY_VERSION_MAJOR")) {
					result.halleyVerMajor = value;
				} else if (line.contains("HALLEY_VERSION_MINOR")) {
					result.halleyVerMinor = value;
				} else if (line.contains("HALLEY_VERSION_REVISION")) {
					result.halleyVerRevision = value;
				}
			}
		}
	}

	return result;
}
