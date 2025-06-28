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
        missile->MissileState = 1; // �߻� ����
        controller.setMissile(missile);
    }
};

TEST_F(MissileControllerTest, SetTarget_SetsDirectionAndTimeCorrectly) {
    Location target = { 38.0, 128.0, 0.0 }; // �ϵ������� 1�� �̵�
    controller.setTarget(target);

    // Target�� ����� �����Ǿ����� Ȯ��
    EXPECT_TRUE(controller.hasTarget());

    // ���� ���� �ð��� 1�� �Ÿ� ���� 111km �̻��̹Ƿ� �뷫 50�� �̻��� ��
    double min_expected_distance_km = 100.0;
    double expected_time = min_expected_distance_km / 2.0; // �ӵ� 2.0 ����

    EXPECT_GE(controller.getEstimatedTimeToImpact(), expected_time);
};