#pragma once

namespace openpsapp::web {
struct Asset {
    const char *path;
    const char *content_type;
    const unsigned char *data;
    unsigned size;
    bool gzip;
};

extern const Asset kAssets[];
extern const unsigned kAssetCount;

}
