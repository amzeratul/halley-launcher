#include "new_version_info.h"

bool NewVersionInfo::isNewVersion() const
{
	return version > currentVersion;
}

NewVersionInfo NewVersionInfo::parse(const Bytes& bytes)
{
	const auto config = YAMLConvert::parseConfig(bytes);
	const auto& root = config.getRoot();

	NewVersionInfo info;
	info.version = root["version"].asInt(0);
	info.download = root["download"].asString("");
	return info;
}
