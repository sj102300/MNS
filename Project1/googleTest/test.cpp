#pragma once
#include "pch.h"
#include "../Project1/add.cpp"
TEST(addtest, 1) {
  ASSERT_NE(5, add(3,2));
  EXPECT_EQ(6, add(3, 4));
  EXPECT_TRUE(true);
}