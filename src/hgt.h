#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include <map>

struct TerrainData {
    int minLat, maxLat;
    int minLon, maxLon;
    int width, height; 
    std::vector<int16_t> fullData;
};

TerrainData loadAllHGT(
    const std::string& folderPath,
    bool useRange,
    int minLon, int maxLon,
    int minLat, int maxLat
);