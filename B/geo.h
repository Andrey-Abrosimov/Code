#pragma once

#include <cmath>

struct Coordinates {
    double lat;
    double lng;
};

inline bool operator== (const Coordinates& r, const Coordinates& l) {
    return r.lat == l.lat && r.lng == l.lng;
}

inline double ComputeDistance(Coordinates from, Coordinates to) {
    using namespace std;
    if (from == to) {
        return 0;
    }
    static const double dr = M_PI / 180.;
    return acos(sin(from.lat * dr) * sin(to.lat * dr)
                + cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr))
        * 6371000;
}
