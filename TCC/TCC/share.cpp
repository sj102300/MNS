#include "share.h"
#include <regex>
#include <cstring>

namespace TCC {

    // Position member functions
    bool Position::isValidPosition(double latitude, double longitude, double altitude) {
        return !(latitude < -90 || latitude > 90 ||
                 longitude < -180 || longitude > 180 ||
                 altitude < 0 || altitude > 10);
    }

    bool Position::isValidLatitude(double latitude) {
        return latitude >= -90 && latitude <= 90;
    }

    bool Position::isValidLongitude(double longitude) {
        return longitude >= -180 && longitude <= 180;
    }

    bool Position::isValidAltitude(double altitude) {
        return altitude >= 0 && altitude <= 10;
    }

    bool Position::isValidPosition() {
        return isValidLatitude(latitude_) &&
               isValidLongitude(longitude_) &&
               isValidAltitude(altitude_);
    }

    bool Position::isZeroVector() {
        return latitude_ == 0 && longitude_ == 0 && altitude_ == 0;
    }

    // Regex validation
    bool isValidAircraftId(const char* input) {
        char tmp[9];
        std::memcpy(tmp, input, 8);
        tmp[8] = '\0';
        std::regex pattern("^ATS-\\d{4}$");
        return std::regex_match(tmp, pattern);
    }

    bool isValidMissileId(const char* input) {
        char tmp[9];
        std::memcpy(tmp, input, 8);
        tmp[8] = '\0';
        std::regex pattern("^MSS-\\d{4}$");
        return std::regex_match(tmp, pattern);
    }

    bool isValidCommandId(const char* input) {
        char tmp[21];
        std::memcpy(tmp, input, 20);
        tmp[20] = '\0'; 
        std::regex pattern("^(AF|MF|ED|SD|SS)-\\d{17}$");
        return std::regex_match(tmp, pattern);
    }

}
