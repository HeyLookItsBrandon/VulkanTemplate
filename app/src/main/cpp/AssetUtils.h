#ifndef ASSETUTILS_H
#define ASSETUTILS_H

#include <vector>
#include <string>
#include <android/asset_manager.h>

std::vector<char> readAsset(AAssetManager *assetManager, std::string assetName);

#endif
