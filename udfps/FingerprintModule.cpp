// SPDX-License-Identifier: Apache-2.0
#include "AncFingerprintDevice.h"
#include "Log.h"
#include "SysfsHbm.h"
#include "UdfpsHandler.h"

#include <atomic>
#include <cerrno>
#include <cstring>
#include <dlfcn.h>
#include <mutex>

#ifndef MEIZU_JIIOV_MODULE_PATH
#define MEIZU_JIIOV_MODULE_PATH "/vendor/lib64/hw/jiiov.fingerprint.vendor.so"
#endif

namespace meizu::fingerprint {
namespace {

struct ModuleState {
  // Serialize calls from the stock service. Native notifications never take
  // this lock: cancel() can wait for a worker that is delivering a callback.
  // Recursive to permit same-thread reentry from the downstream notification.
  std::recursive_mutex calls;
  UdfpsHandler udfps{writeHbm};
  std::atomic<AncNotify> notify{nullptr};
  void *library = nullptr;
  hw_module_t *module = nullptr;
  AncFingerprintDevice *device = nullptr;
  AncFingerprintDevice original{};
};

ModuleState &state() {
  // The library and callback state live for the HAL process lifetime. Do not
  // dlclose the vendor library while its worker threads may still be alive.
  static ModuleState instance;
  return instance;
}

void onNotify(const AncMessage *message) {
  if (message == nullptr)
    return;
  auto &s = state();
  const bool terminal = message->type == -1 || message->type == 100 ||
                        message->type == 101 ||
                        (message->type == 5 && message->data[0] != 0) ||
                        (message->type == 3 && message->data[1] == 0);
  if (terminal) {
    // Invalidate a TouchDown that is still inside the vendor call before
    // forwarding the completion to the framework.
    s.udfps.finishOperation();
  }
  if (auto callback = s.notify.load(std::memory_order_acquire))
    callback(message);
}

int setNotify(AncFingerprintDevice *device, AncNotify callback) {
  auto &s = state();
  std::lock_guard lock(s.calls);
  if (device != s.device || !callback)
    return -EINVAL;
  auto previous = s.notify.exchange(callback, std::memory_order_acq_rel);
  const int ret = s.original.set_notify(device, onNotify);
  if (ret != 0)
    s.notify.store(previous, std::memory_order_release);
  return ret;
}

int enroll(AncFingerprintDevice *device, const AncAuthToken *hat) {
  auto &s = state();
  std::lock_guard lock(s.calls);
  if (device != s.device)
    return -EINVAL;
  const auto operation = s.udfps.beginOperation();
  const int ret = s.original.enroll(device, hat);
  if (ret != 0)
    s.udfps.failOperation(operation);
  return ret;
}

int authenticate(AncFingerprintDevice *device, uint64_t operationId) {
  auto &s = state();
  std::lock_guard lock(s.calls);
  if (device != s.device)
    return -EINVAL;
  const auto operation = s.udfps.beginOperation();
  const int ret = s.original.authenticate(device, operationId);
  if (ret != 0)
    s.udfps.failOperation(operation);
  return ret;
}

int cancel(AncFingerprintDevice *device) {
  auto &s = state();
  std::lock_guard lock(s.calls);
  if (device != s.device)
    return -EINVAL;
  s.udfps.finishOperation();
  return s.original.cancel(device);
}

int setActiveGroup(AncFingerprintDevice *device, uint32_t group,
                   const char *path) {
  auto &s = state();
  std::lock_guard lock(s.calls);
  if (device != s.device)
    return -EINVAL;
  s.udfps.finishOperation();
  return s.original.set_active_group(device, group, path);
}

int sendCommand(AncFingerprintDevice *device, int32_t command, int32_t arg,
                const uint8_t *payload, uint32_t size) {
  auto &s = state();
  std::lock_guard lock(s.calls);
  if (device != s.device)
    return -EINVAL;

  if (command == kTouchDown) {
    auto touch = s.udfps.beginTouch();
    if (!touch)
      return 0;
    const int ret =
        s.original.send_command(device, command, arg, payload, size);
    if (ret != 0) {
      s.udfps.endTouch();
      return ret;
    }
    // This happens before returning to FingerprintControl, which only then
    // schedules the existing hbm_ready_status polling work item.
    if (!s.udfps.illuminate(*touch)) {
      FP_LOGE("TouchDown ended or HBM enable failed");
      return -EIO;
    }
    return 0;
  }

  if (command == kTouchUp) {
    // Always release the panel, even if the vendor operation later fails.
    s.udfps.endTouch();
  } else if (command == kHbmReady && !s.udfps.consumeReady()) {
    return 0;
  }

  const int ret = s.original.send_command(device, command, arg, payload, size);
  if (command == kHbmReady && ret != 0)
    s.udfps.endTouch();
  return ret;
}

int closeDevice(hw_device_t *device) {
  auto &s = state();
  std::lock_guard lock(s.calls);
  if (!s.device || device != &s.device->common)
    return -EINVAL;
  s.udfps.finishOperation();
  // Stock FingerprintClose compares this address against its global object.
  const int ret = s.original.common.close(device);
  // This blob zeroes its global device even when DeinitFingerprintDevice
  // returns an error. Never leave a pointer to that cleared function table.
  s.device = nullptr;
  s.notify.store(nullptr, std::memory_order_release);
  return ret;
}

int openDevice(const hw_module_t *, const char *id, hw_device_t **out) {
  if (!out)
    return -EINVAL;
  *out = nullptr;
  auto &s = state();
  std::lock_guard lock(s.calls);
  if (s.device)
    return -EBUSY;
  s.udfps.finishOperation();

  if (!s.library) {
    s.library = dlopen(MEIZU_JIIOV_MODULE_PATH, RTLD_NOW | RTLD_LOCAL);
    if (!s.library) {
      FP_LOGE("dlopen %s: %s", MEIZU_JIIOV_MODULE_PATH, dlerror());
      return -ENOENT;
    }
    s.module = static_cast<hw_module_t *>(
        dlsym(s.library, HAL_MODULE_INFO_SYM_AS_STR));
  }
  if (!s.module || s.module->tag != HARDWARE_MODULE_TAG || !s.module->id ||
      std::strcmp(s.module->id, "jiiov.fingerprint") != 0 ||
      !s.module->methods || !s.module->methods->open ||
      s.module->methods->open == openDevice) {
    FP_LOGE("Invalid stock JIIOV module; check the renamed blob and SONAME");
    return -EINVAL;
  }

  hw_device_t *real = nullptr;
  const int ret = s.module->methods->open(s.module, id, &real);
  if (ret != 0)
    return ret;
  if (!real || real->tag != HARDWARE_DEVICE_TAG || real->version != 0x201) {
    FP_LOGE("Unexpected stock JIIOV device/version");
    if (real && real->close)
      real->close(real);
    return -EINVAL;
  }
  auto *device = reinterpret_cast<AncFingerprintDevice *>(real);
  if (!device->common.close || !device->set_notify || !device->enroll ||
      !device->authenticate || !device->cancel || !device->set_active_group ||
      !device->send_command || !device->inner) {
    FP_LOGE("Incomplete ANC device function table");
    if (real->close)
      real->close(real);
    return -EINVAL;
  }

  s.original = *device;
  s.device = device;
  s.notify.store(nullptr, std::memory_order_release);
  // Patch only known slots. All other entry points and the original device
  // pointer are preserved, including the private tail used by the blob.
  device->common.close = closeDevice;
  device->set_notify = setNotify;
  device->enroll = enroll;
  device->authenticate = authenticate;
  device->cancel = cancel;
  device->set_active_group = setActiveGroup;
  device->send_command = sendCommand;
  *out = real;
  FP_LOGI("JIIOV wrapper loaded; operation-gated HBM enabled");
  return 0;
}

hw_module_methods_t methods = {openDevice};

} // namespace
} // namespace meizu::fingerprint

extern "C" {
__attribute__((visibility("default"))) hw_module_t HAL_MODULE_INFO_SYM = {
    .tag = HARDWARE_MODULE_TAG,
    .module_api_version = HARDWARE_MODULE_API_VERSION(2, 1),
    .hal_api_version = HARDWARE_HAL_API_VERSION,
    .id = "jiiov.fingerprint",
    .name = "Meizu JIIOV UDFPS wrapper",
    .author = "The LineageOS Project",
    .methods = &meizu::fingerprint::methods,
    .dso = nullptr,
    .reserved = {},
};
}
