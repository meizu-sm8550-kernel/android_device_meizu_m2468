// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <hardware/hardware.h>

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace meizu::fingerprint {

// Private arm64 ABI from Meizu's JIIOV module, NOT hardware/fingerprint.h.
// These messages are borrowed from the blob. Only inspect the prefix; always
// forward the original pointer, including the opaque tail (HAT, ID array,
// etc.).
struct AncMessage {
  int32_t type;
  uint32_t padding;
  uint32_t data[2];
};
using AncNotify = void (*)(const AncMessage *);
struct AncAuthToken; // Opaque: the stock AIDL service owns token conversion.

struct AncFingerprintDevice {
  hw_device_t common;
  AncNotify notify;
  int (*set_notify)(AncFingerprintDevice *, AncNotify);
  int (*generate_challenge)(AncFingerprintDevice *);
  int (*revoke_challenge)(AncFingerprintDevice *, uint64_t);
  int (*enroll)(AncFingerprintDevice *, const AncAuthToken *);
  int (*get_authenticator_id)(AncFingerprintDevice *);
  int (*invalidate_authenticator_id)(AncFingerprintDevice *);
  int (*cancel)(AncFingerprintDevice *);
  int (*enumerate)(AncFingerprintDevice *);
  int (*remove)(AncFingerprintDevice *, const uint32_t *, uint32_t);
  int (*set_active_group)(AncFingerprintDevice *, uint32_t, const char *);
  int (*authenticate)(AncFingerprintDevice *, uint64_t);
  void (*on_pointer_down)(AncFingerprintDevice *, int32_t, int32_t, int32_t,
                          float, float);
  void (*on_pointer_up)(AncFingerprintDevice *, int32_t);
  int (*get_sensor_type)();
  int (*send_command)(AncFingerprintDevice *, int32_t, int32_t, const uint8_t *,
                      uint32_t);
  int (*reset_lockout)(AncFingerprintDevice *, const AncAuthToken *);
  void *reserved[4];
  void *inner;
};

static_assert(sizeof(void *) == 8, "The Meizu ANC ABI is arm64 only");
static_assert(std::is_standard_layout_v<AncFingerprintDevice>);
static_assert(sizeof(hw_device_t) == 0x78);
static_assert(offsetof(AncMessage, data) == 8);
#define ANC_OFFSET(member, offset)                                             \
  static_assert(offsetof(AncFingerprintDevice, member) == offset)
ANC_OFFSET(common, 0x00);
ANC_OFFSET(notify, 0x78);
ANC_OFFSET(set_notify, 0x80);
ANC_OFFSET(generate_challenge, 0x88);
ANC_OFFSET(revoke_challenge, 0x90);
ANC_OFFSET(enroll, 0x98);
ANC_OFFSET(get_authenticator_id, 0xa0);
ANC_OFFSET(invalidate_authenticator_id, 0xa8);
ANC_OFFSET(cancel, 0xb0);
ANC_OFFSET(enumerate, 0xb8);
ANC_OFFSET(remove, 0xc0);
ANC_OFFSET(set_active_group, 0xc8);
ANC_OFFSET(authenticate, 0xd0);
ANC_OFFSET(on_pointer_down, 0xd8);
ANC_OFFSET(on_pointer_up, 0xe0);
ANC_OFFSET(get_sensor_type, 0xe8);
ANC_OFFSET(send_command, 0xf0);
ANC_OFFSET(reset_lockout, 0xf8);
ANC_OFFSET(inner, 0x120);
#undef ANC_OFFSET
static_assert(sizeof(AncFingerprintDevice) == 0x128);

constexpr int32_t kTouchDown = 20013;
constexpr int32_t kTouchUp = 20014;
constexpr int32_t kHbmReady = 20015;

} // namespace meizu::fingerprint
