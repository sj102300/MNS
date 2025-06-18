#include "Aircraft.h"

Aircraft::Aircraft(const std::string& id, const Location& location)
    : id_(id), location_(location) {}

//void Aircraft::update(const Location& newLocation, const Velocity& newVelocity) {
//    location_ = newLocation;
//    velocity_ = newVelocity;
//}
void Aircraft::update(const Location& newLocation) {
       location_ = newLocation;
       //velocity_ = newVelocity;
}

const std::string& Aircraft::getId() const {
    return id_;
}

const Location& Aircraft::getLocation() const {
    return location_;
}

const Velocity& Aircraft::getVelocity() const {
    return velocity_;
}
