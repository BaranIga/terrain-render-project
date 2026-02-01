#include "hgt.h"
#include <fstream>
#include <iostream>
#include <filesystem> 
#include <regex>

namespace fs = std::filesystem;

constexpr int16_t NO_DATA = -32768;

struct TileRef {
    std::string path;
    int lat;
    int lon;
};

static int16_t readBigEndianInt16(std::ifstream& f) {
    unsigned char hi, lo;
    f.read((char*)&hi, 1);
    f.read((char*)&lo, 1);
    return (int16_t)((hi << 8) | lo);
}

TerrainData loadAllHGT(
    const std::string& folderPath,
    bool useRange,
    int reqMinLon, int reqMaxLon,
    int reqMinLat, int reqMaxLat
) {
    TerrainData td;
    struct TileRef { std::string path; int lat, lon; };
    std::vector<TileRef> tiles;

    int minLat = 90, maxLat = -90;
    int minLon = 180, maxLon = -180;

    std::regex pattern("([NS])(\\d{2})([EW])(\\d{3})\\.hgt");

    for (const auto& entry : fs::directory_iterator(folderPath)) {
        std::string filename = entry.path().filename().string();
        std::smatch match;

        if (!std::regex_match(filename, match, pattern))
            continue;

        int lat = std::stoi(match[2]) * (match[1] == "S" ? -1 : 1);
        int lon = std::stoi(match[4]) * (match[3] == "W" ? -1 : 1);

        if (useRange) {
            if (lon < reqMinLon || lon > reqMaxLon ||
                lat < reqMinLat || lat > reqMaxLat)
                continue;
        }

        tiles.push_back(TileRef{ entry.path().string(), lat, lon });

        minLat = std::min(minLat, lat);
        maxLat = std::max(maxLat, lat);
        minLon = std::min(minLon, lon);
        maxLon = std::max(maxLon, lon);
    }

    if (tiles.empty()) 
        return td;

    int latCount = maxLat - minLat + 1;
    int lonCount = maxLon - minLon + 1;
    td.width = lonCount * 1200 + 1;
    td.height = latCount * 1200 + 1;
    td.fullData.resize(td.width * td.height, 0);
    
    td.minLat = minLat;
    td.maxLat = maxLat;
    td.minLon = minLon;
    td.maxLon = maxLon;

    constexpr int16_t NO_DATA = -32768;

    for (const auto& tile : tiles) {
        std::ifstream file(tile.path, std::ios::binary);
        if (!file) continue;

        int startX = (tile.lon - minLon) * 1200;
        int startY = (maxLat - tile.lat) * 1200;

        for (int i = 0; i < 1201; i++) {
            for (int j = 0; j < 1201; j++) {
                int16_t val = readBigEndianInt16(file);

                if (val < -500 || val > 9000) {
                    val = 0;
                }

                int globalX = startX + j;
                int globalY = startY + i;
                td.fullData[globalY * td.width + globalX] = val;
            }
        }
    }
    return td;
}