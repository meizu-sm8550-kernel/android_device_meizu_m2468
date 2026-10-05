// SPDX-License-Identifier: Apache-2.0
#pragma once

#ifdef __ANDROID__
#include <android/log.h>
#define FP_LOGE(...)                                                           \
  __android_log_print(ANDROID_LOG_ERROR, "MeizuUdfps", __VA_ARGS__)
#define FP_LOGI(...)                                                           \
  __android_log_print(ANDROID_LOG_INFO, "MeizuUdfps", __VA_ARGS__)
#else
#include <cstdio>
#define FP_LOGE(...)                                                           \
  do {                                                                         \
    std::fprintf(stderr, "MeizuUdfps: ");                                      \
    std::fprintf(stderr, __VA_ARGS__);                                         \
    std::fputc('\n', stderr);                                                  \
  } while (0)
#define FP_LOGI(...) FP_LOGE(__VA_ARGS__)
#endif
