#include "launcher_project_properties.h"

std::optional<LauncherProjectProperties> LauncherProjectProperties::getProjectProperties(const ProjectLocation& project, Resources* resources, VideoAPI* videoAPI)
{
	const auto& path = Path(project.path);

	const auto propertiesBytes = Path::readFile(path / "halley_project" / "properties.yaml");
	if (propertiesBytes.empty()) {
		return {};
	}

	LauncherProjectProperties result;

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

	const auto halleyVersionPath = path / "halley" / "include" / "halley_version.hpp";
	if (Path::exists(halleyVersionPath)) {
		result.halleyVersion.parseHeader(Path::readFileLines(halleyVersionPath));
	} else {
		result.halleyVersion.parse(config.getRoot()["halleyVersion"].asString("0.0.0"));
	}

	if (Path::exists(path / "halley" / "bin" / "halley-editor.exe")) {
		result.builtVersion.parse(Path::readFileString(path / "halley" / "bin" / "build_version.txt"));
	}

	result.cleanBuildIfOlderVersion.parse(Path::readFileString(path / "halley" / "include" / "clean_build_if_older.txt"));

	return result;
}
