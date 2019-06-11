// Copyright 2019 Pokitec
// All rights reserved.

#include <TTauri/Color.hpp>
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace TTauri;

TEST(ColorTests, initialize) {
    ASSERT_EQ(wsRGBApm{ 0x123456ff }.to_sRGBA_u32(), 0x123456ff);
    ASSERT_EQ(wsRGBApm{ 0x6889abcd }.to_sRGBA_u32(), 0x6889abcd);
}
