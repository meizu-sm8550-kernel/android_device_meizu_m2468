// SPDX-License-Identifier: Apache-2.0
#include "UdfpsHandler.h"
#include <utility>

namespace meizu::fingerprint {

UdfpsHandler::UdfpsHandler(Writer writer) : mWriter(std::move(writer)) {
  offLocked();
}

bool UdfpsHandler::offLocked() {
  mIlluminated = false;
  if (mMayBeOn && !mWriter(false))
    return false;
  mMayBeOn = false;
  return true;
}

void UdfpsHandler::endTouchLocked() {
  ++mPress;
  mPressed = false;
  mReadySent = false;
  offLocked();
}

uint64_t UdfpsHandler::beginOperation() {
  std::lock_guard lock(mMutex);
  endTouchLocked();
  mActive = true;
  return ++mOperation;
}

void UdfpsHandler::failOperation(uint64_t operation) {
  std::lock_guard lock(mMutex);
  if (operation != mOperation)
    return;
  mActive = false;
  endTouchLocked();
}

void UdfpsHandler::finishOperation() {
  std::lock_guard lock(mMutex);
  mActive = false;
  endTouchLocked();
}

std::optional<UdfpsHandler::Touch> UdfpsHandler::beginTouch() {
  std::lock_guard lock(mMutex);
  if (!mActive || mPressed || !offLocked())
    return std::nullopt;
  mPressed = true;
  mReadySent = false;
  return Touch{mOperation, ++mPress};
}

bool UdfpsHandler::illuminate(Touch touch) {
  std::lock_guard lock(mMutex);
  if (!mActive || !mPressed || touch.operation != mOperation ||
      touch.press != mPress) {
    return false;
  }
  if (mIlluminated)
    return true;
  // Even a failed/short sysfs write may have affected the panel.
  mMayBeOn = true;
  if (!mWriter(true)) {
    offLocked();
    return false;
  }
  mIlluminated = true;
  return true;
}

bool UdfpsHandler::consumeReady() {
  std::lock_guard lock(mMutex);
  if (!mActive || !mPressed || !mIlluminated || mReadySent)
    return false;
  mReadySent = true;
  return true;
}

void UdfpsHandler::endTouch() {
  std::lock_guard lock(mMutex);
  endTouchLocked();
}

} // namespace meizu::fingerprint
