#include "pch.h"
#include <gtest/gtest.h>
#include "../MSS/Missile.cpp"
#include "../MSS/MissileController.cpp"
#include "../MSS/UdpMulticast.cpp"
#include "../MSS/UdpReceiver.cpp"


class MissileControllerTest : public ::testing::Test {
protected:
    std::shared_ptr<Missile> missile;
    MissileController controller;

    void SetUp() override {
        missile = std::make_shared<Missile>();
        missile->MissileLoc = { 37.0, 127.0, 0.0 };
        missile->MissileState = 1; // 발사 상태
        controller.setMissile(missile);
    }
};

TEST_F(MissileControllerTest, SetTarget_SetsDirectionAndTimeCorrectly) {
    Location target = { 38.0, 128.0, 0.0 }; // 북동쪽으로 1도 이동
    controller.setTarget(target);

    // Target이 제대로 설정되었는지 확인
    EXPECT_TRUE(controller.hasTarget());

    // 예상 도달 시간은 1도 거리 기준 111km 이상이므로 대략 50초 이상이 됨
    double min_expected_distance_km = 100.0;
    double expected_time = min_expected_distance_km / 2.0; // 속도 2.0 기준

    EXPECT_GE(controller.getEstimatedTimeToImpact(), expected_time);
};