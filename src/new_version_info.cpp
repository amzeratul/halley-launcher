#include "new_version_info.h"

bool NewVersionInfo::isNewVersion() const
{
	return version > currentVersion;
}

NewVersionInfo NewVersionInfo::parse(const Bytes& bytes)
{
	const auto config = YAMLConvert::parseConfig(bytes);
	const auto& root = config.getRoot();

	String myPlatform;
	if constexpr (getPlatform() == GamePlatform::Windows) {
		myPlatform = "win64";
	} else if constexpr (getPlatform() == GamePlatform::MacOS) {
		myPlatform = "macos";
	} else if constexpr (getPlatform() == GamePlatform::Linux) {
		myPlatform = "linux";
	}

	NewVersionInfo info;
	info.version = root["version"].asInt(0);
	if (root.hasKey("download")) {
		const auto& version = root["download"][myPlatform];
		if (version.getType() != ConfigNodeType::Undefined) {
			info.downloadURL = version["url"].asString("");
			if (version.hasKey("signature")) {
				info.signature = version["signature"].asBytes();
			}
		}
	}
	return info;
}
