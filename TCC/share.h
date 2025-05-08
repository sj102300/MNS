#pragma once

namespace TCC {
    typedef struct _position {
        double latitude_;
        double longitude_;
        double altitude_;
        bool isValidPosition(double latitude, double longitude, double altitude) {
            if (latitude < -90 || latitude > 90 ||
                longitude < -180 || longitude > 180 ||
                altitude < 0 || altitude > 10)
                return false;
            return true;
        }
        bool isValidLatitude(double latitude) {
            if (latitude < -90 || latitude > 90)    return false;
            return true;
        }
        bool isValidLongitude(double longitude) {
            if (longitude < -180 || longitude > 180)    return false;
            return true;
        }
        bool isValidAltitude(double altitude) {
            if (altitude < 0 || altitude>10)   return false;
            return true;
        }
    } Position;
}