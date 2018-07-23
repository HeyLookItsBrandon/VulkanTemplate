#include "AssetUtils.h"

std::vector<char> readAsset(AAssetManager *assetManager, std::string assetName) {
	AAsset* asset = AAssetManager_open(assetManager,
			assetName.c_str(), AASSET_MODE_BUFFER);
	if(asset == nullptr) {
		std::string message = std::string("Asset ") += assetName += " could not be opened.";
		throw std::runtime_error(message);
	}

	off_t assetLength = AAsset_getLength(asset);

	std::vector<char> contents(assetLength);
	AAsset_read(asset, contents.data(), assetLength);

	AAsset_close(asset);

	return contents;
}
