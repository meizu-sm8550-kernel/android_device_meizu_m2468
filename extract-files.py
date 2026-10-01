#!/usr/bin/env -S PYTHONPATH=../../../tools/extract-utils python3
#
# SPDX-FileCopyrightText: 2024 The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

from extract_utils.file import File
from extract_utils.fixups_blob import (
    BlobFixupCtx,
    blob_fixup,
    blob_fixups_user_type,
)
from extract_utils.fixups_lib import (
    lib_fixups,
    lib_fixups_user_type,
)
from extract_utils.main import (
    ExtractUtils,
    ExtractUtilsModule,
)

namespace_imports = [
    'device/meizu/m2468',
    'hardware/qcom-caf/wlan',
    'hardware/qcom-caf/sm8550',
    'vendor/qcom/opensource/commonsys-intf/display',
    'vendor/qcom/opensource/dataservices',
]

def lib_fixup_vendor_suffix(lib: str, partition: str, *args, **kwargs):
    return f'{lib}-{partition}' if partition == 'vendor' else None


lib_fixups: lib_fixups_user_type = {
    **lib_fixups,
    (

    ): lib_fixup_vendor_suffix,
}

blob_fixups: blob_fixups_user_type = {
    (
    'product/etc/permissions/vendor.qti.hardware.data.connection-V1.0-java.xml',
    'product/etc/permissions/vendor.qti.hardware.data.connection-V1.1-java.xml',
    'product/etc/permissions/vendor.qti.hardware.data.connectionaidl-V1-java.xml',
    ): blob_fixup()
        .regex_replace('version="2.0"', 'version="1.0"'),
    (
    'vendor/bin/hw/android.hardware.security.keymint-service-qti',
    'vendor/bin/hw/android.hardware.security.keymint-service-spu-qti'
    ): blob_fixup()
        .add_needed('android.hardware.security.rkp-V3-ndk.so'),
    (
    'vendor/lib64/libspukeymint.so',
    'vendor/lib64/libqtikeymint.so',
    ): blob_fixup()
        .add_needed('android.hardware.security.rkp-V3-ndk.so'),
    'vendor/lib64/vendor.libdpmframework.so': blob_fixup()
        .add_needed('libhidlbase_shim.so'),
    (
    'vendor/bin/qguard',
    'vendor/lib64/tms-utils.so',
    'vendor/lib64/nfc_nci.thn31nfc.tms.so',
    'vendor/lib64/nfc_nci.nqx.default.hw.so',
    ): blob_fixup()
        .add_needed('libbase_shim.so'),
    'vendor/lib64/libqcodec2_core.so': blob_fixup()
        .add_needed('qcodec2_shim.so'),
    (
    'vendor/lib64/libbluetooth_audio_session_aidl.so',
    'vendor/lib64/libbluetooth_audio_session_aidl_qti.so',
    'vendor/lib64/hw/android.hardware.bluetooth.audio-impl-qti.so',
    'vendor/lib64/hw/audio.bluetooth.default.so',
    'vendor/lib64/hw/audio.bluetooth_qti.default.so',
    'vendor/lib64/btaudio_offload_if.so',
    ): blob_fixup()
        .replace_needed('android.hardware.bluetooth.audio-V2-ndk.so','android.hardware.bluetooth.audio-V5-ndk.so'),
    'system_ext/lib64/vendor.qti.hardware.qccsyshal@1.2-halimpl.so' : blob_fixup()
        .replace_needed('libprotobuf-cpp-full.so', 'libprotobuf-cpp-full-21.7.so'),
    'vendor/lib64/libarcsoft_high_dynamic_range_v5.so': blob_fixup()
        .clear_symbol_version('rpcmem_alloc')
        .clear_symbol_version('rpcmem_free')
        .clear_symbol_version('rpcmem_to_fd'),
    (
    'vendor/bin/slim_daemon'
    ): blob_fixup()
        .add_needed('libemutls_get_address.so'),
    (
    'vendor/lib64/libdpps.so',
    'vendor/lib64/libsnapdragoncolor-manager.so',
    'vendor/lib64/libsynclight.so',
    ): blob_fixup()
        .replace_needed('libtinyxml2.so', 'libtinyxml2_1.so'),
}  # fmt: skip

module = ExtractUtilsModule(
    'm2468',
    'meizu',
    blob_fixups=blob_fixups,
    lib_fixups=lib_fixups,
    namespace_imports=namespace_imports,
)

if __name__ == '__main__':
    utils = ExtractUtils.device(module)
    utils.run()
