# Meizu 21 Note UDFPS

`udfps/` builds `jiiov.fingerprint.default.so`, a wrapper around the stock ANC/JIIOV module. The stock AIDL service, input reader, ready polling, TA and calibration data remain in use.

The wrapper enables `/sys/kernel/display_drivers/hbm` with ASCII `6` only after an accepted TouchDown (20013) during authenticate/enroll. TouchUp (20014), cancellation, user switching and terminal notifications restore ASCII `7`. It forwards the stock service's HBMReady (20015) only while the current press is illuminated. No `/proc` probing or extra input daemon is used.

The function table in `AncFingerprintDevice.h` is specific to this device's arm64 ANC blob. Its layout and callback payloads differ from `hardware/fingerprint.h`; do not replace it with the standard legacy structure. The original device pointer is preserved because the blob checks its identity on close.

## Vendor extraction and build

After updating the device tree, **re-extract blobs and regenerate the vendor build files** with the new `proprietary-files.txt`. Its `source:destination;FIX_SONAME` entry installs the original module as:

```text
/vendor/lib64/hw/jiiov.fingerprint.vendor.so
DT_SONAME = jiiov.fingerprint.vendor.so
```

The source wrapper installs at the original path:

```text
/vendor/lib64/hw/jiiov.fingerprint.default.so
```

An old generated vendor tree still declaring the prebuilt `jiiov.fingerprint.default` will conflict with the new source module. Do not copy the original blob over the wrapper. Both files are required in the final image.

In an initialized LineageOS 24.0 build environment with the m2468 product selected, build the wrapper with:

```sh
m jiiov.fingerprint.default
```

Build/flash an updated vendor image to include the renamed original, init changes and SELinux changes. The old `mz_fp_hbm_daemon` package/service has been removed. The original `anc_fps` service and binary gesture initialization remain enabled as before.

## Diagnostics

The wrapper logs under `MeizuUdfps`:

```sh
adb logcat -s MeizuUdfps ANC_AIDL
```

Successful loading logs `JIIOV wrapper loaded; operation-gated HBM enabled`. Press/release should produce `HBM=6` / `HBM=7`; the stock service should then report `wait_hbm_on success`. Check enrollment, lockscreen, app prompts, quick release, cancellation while pressed, failed-match retries, and AOD independently.

If loading fails, check the renamed file, its SONAME, and its dependencies. If node access fails, check ownership and AVCs. The included SELinux rules cover the HBM nodes and assign the stock executable to `hal_fingerprint_default`; other stock fingerprint/TEE permissions still depend on the ROM's existing policy. This change does not switch the device tree out of its existing permissive configuration.

The original service's input thread and ready polling are retained, including their cancellation/queue behavior. Init restores `7` when `anc_fps` stops or restarts. Failed HBM-off writes remain dirty and are retried on subsequent cleanup/press events; node errors are logged rather than treated as successful transitions.

Local lifecycle/concurrency verification and an NDK arm64 compile were performed. The standalone NDK artifact is not a ROM-installable replacement (its C++ runtime differs from Soong); build the sources in the ROM. A complete Soong/SELinux image build and real-panel verification remain required. Verification fixtures are intentionally not shipped in this device tree.
