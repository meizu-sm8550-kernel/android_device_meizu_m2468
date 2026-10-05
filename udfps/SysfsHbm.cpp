// SPDX-License-Identifier: Apache-2.0
#include "SysfsHbm.h"
#include "Log.h"

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

#ifndef MEIZU_HBM_PATH
#define MEIZU_HBM_PATH "/sys/kernel/display_drivers/hbm"
#endif

namespace meizu::fingerprint {

bool writeHbm(bool enabled) {
  int fd;
  do {
    fd = open(MEIZU_HBM_PATH, O_WRONLY | O_CLOEXEC);
  } while (fd < 0 && errno == EINTR);
  if (fd < 0) {
    FP_LOGE("open HBM for %s: %s", enabled ? "on" : "off", strerror(errno));
    return false;
  }

  const char value = enabled ? '6' : '7';
  ssize_t size;
  do {
    size = write(fd, &value, 1);
  } while (size < 0 && errno == EINTR);
  const int error = errno;
  close(fd);
  if (size != 1) {
    FP_LOGE("write HBM=%c: %s", value,
            size < 0 ? strerror(error) : "short write");
    return false;
  }
  FP_LOGI("HBM=%c", value);
  return true;
}

} // namespace meizu::fingerprint
