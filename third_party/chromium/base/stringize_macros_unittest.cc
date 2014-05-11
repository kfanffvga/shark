// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Unit tests for stringize_macros.h

#include "base/stringize_macros.h"

#include "testing/gtest/include/gtest/gtest.h"

// Macros as per documentation in header file.
#define PREPROCESSOR_UTIL_UNITTEST_A FOO
#define PREPROCESSOR_UTIL_UNITTEST_B(x) myobj->FunctionCall(x)
#define PREPROCESSOR_UTIL_UNITTEST_C "foo"

TEST(StringizeTest, Ansi) {
  EXPECT_STREQ(
      "PREPROCESSOR_UTIL_UNITTEST_A",
      STRINGIZE_NO_EXPANSION(PREPROCESSOR_UTIL_UNITTEST_A));
  EXPECT_STREQ(
      "PREPROCESSOR_UTIL_UNITTEST_B(y)",
      STRINGIZE_NO_EXPANSION(PREPROCESSOR_UTIL_UNITTEST_B(y)));
  EXPECT_STREQ(
      "PREPROCESSOR_UTIL_UNITTEST_C",
      STRINGIZE_NO_EXPANSION(PREPROCESSOR_UTIL_UNITTEST_C));

  EXPECT_STREQ("FOO", STRINGIZE(PREPROCESSOR_UTIL_UNITTEST_A));
  EXPECT_STREQ("myobj->FunctionCall(y)",
               STRINGIZE(PREPROCESSOR_UTIL_UNITTEST_B(y)));
  EXPECT_STREQ("\"foo\"", STRINGIZE(PREPROCESSOR_UTIL_UNITTEST_C));
}

#if defined(OS_WIN)

TEST(StringizeTest, Wide) {
  EXPECT_STREQ(
      L"PREPROCESSOR_UTIL_UNITTEST_A",
      LSTRINGIZE_NO_EXPANSION(PREPROCESSOR_UTIL_UNITTEST_A));
  EXPECT_STREQ(
      L"PREPROCESSOR_UTIL_UNITTEST_B(y)",
      LSTRINGIZE_NO_EXPANSION(PREPROCESSOR_UTIL_UNITTEST_B(y)));
  EXPECT_STREQ(
      L"PREPROCESSOR_UTIL_UNITTEST_C",
      LSTRINGIZE_NO_EXPANSION(PREPROCESSOR_UTIL_UNITTEST_C));

  EXPECT_STREQ(L"FOO", LSTRINGIZE(PREPROCESSOR_UTIL_UNITTEST_A));
  EXPECT_STREQ(L"myobj->FunctionCall(y)",
               LSTRINGIZE(PREPROCESSOR_UTIL_UNITTEST_B(y)));
  EXPECT_STREQ(L"\"foo\"", LSTRINGIZE(PREPROCESSOR_UTIL_UNITTEST_C));
}

TEST(ToLStringTest, Main) {
  EXPECT_STREQ(L"blat", TO_L_STRING_NO_EXPANSION("blat"));

  EXPECT_STREQ(L"foo", TO_L_STRING(PREPROCESSOR_UTIL_UNITTEST_C));
  EXPECT_STREQ(L"blat", TO_L_STRING("blat"));
}

#endif  // defined(OS_WIN)
