// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <cstdint>
#include <functional>
#include <mutex>
#include <optional>

namespace meizu::fingerprint {

class UdfpsHandler {
public:
  using Writer = std::function<bool(bool)>;
  struct Touch {
    uint64_t operation;
    uint64_t press;
  };

  explicit UdfpsHandler(Writer writer);
  uint64_t beginOperation();
  void failOperation(uint64_t operation);
  void finishOperation();
  std::optional<Touch> beginTouch();
  bool illuminate(Touch touch);
  bool consumeReady();
  void endTouch();

private:
  bool offLocked();
  void endTouchLocked();
  Writer mWriter;
  std::mutex mMutex;
  uint64_t mOperation = 0;
  uint64_t mPress = 0;
  bool mActive = false;
  bool mPressed = false;
  bool mIlluminated = false;
  bool mMayBeOn = true;
  bool mReadySent = false;
};

} // namespace meizu::fingerprint
