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
    # Stock ANC AIDL V1: route SystemUI pointer events to the device callbacks.
    # Each signature is unique in the verified Meizu 21 Note service binary.
    # Patch the entry/logging blocks only; preserve PAC, stack and return code.
    # The bridge also re-arms/stops FingerprintControl's one-shot ready poller.
    'vendor/bin/hw/android.hardware.biometrics.fingerprint-service-jv': blob_fixup()
        # onPointerDown entry @ 0xd830
        .sig_replace(
            'C1 FF FF 90 40 00 80 52 21 B4 3A 91 F3 03 08 AA 5C 22 00 94 00 01 00 37',
            'F3 03 08 AA 0C 00 00 14 ' + '1F 20 03 D5 ' * 4,
        )
        # onPointerDown bridge @ 0xd864
        .sig_replace(
            '59 22 00 94 C1 FF FF B0 C4 FF FF 90 F4 03 00 AA',
            '69 00 00 90 29 1D 40 F9 E9 FE FF B4 20 05 40 F9 A0 FE FF B4 '
            '09 6C 40 F9 69 FE FF B4 20 01 3F D6 99 16 00 94 F0 FF FF 17 '
            + '1F 20 03 D5 ' * 12,
        )
        # onPointerUp entry @ 0xd8d0
        .sig_replace(
            'C1 FF FF 90 40 00 80 52 21 B4 3A 91 F3 03 08 AA 34 22 00 94 00 01 00 37',
            'F3 03 08 AA 0C 00 00 14 ' + '1F 20 03 D5 ' * 4,
        )
        # onPointerUp bridge @ 0xd904
        .sig_replace(
            '31 22 00 94 C1 FF FF B0 C4 FF FF 90 F4 03 00 AA',
            '69 00 00 90 29 1D 40 F9 E9 FE FF B4 20 05 40 F9 A0 FE FF B4 '
            '09 70 40 F9 69 FE FF B4 20 01 3F D6 C1 18 00 94 F0 FF FF 17 '
            + '1F 20 03 D5 ' * 12,
        ),
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
    ): blob_fixup()
        .add_needed('libbase_shim.so'),
    'vendor/lib64/libqcodec2_core.so': blob_fixup()
        .add_needed('qcodec2_shim.so'),
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
    (
        'vendor/bin/poweropt-service',
        'vendor/lib64/libaodoptfeature.so',
        'vendor/lib64/libapengine.so',
        'vendor/lib64/libgamepoweroptfeature.so',
        'vendor/lib64/liblearningmodule.so',
        'vendor/lib64/liboffscreenpoweroptfeature.so',
        'vendor/lib64/libpowercallback.so',
        'vendor/lib64/libpowercore.so',
        'vendor/lib64/libpsmoptfeature.so',
        'vendor/lib64/libstandbyfeature.so',
        'vendor/lib64/libvideooptfeature.so',
    ): blob_fixup()
        .replace_needed('libtinyxml2.so','libtinyxml2_1.so'),

    'vendor/lib64/libarcsoft_beautyshot.so': blob_fixup()
        .clear_symbol_version('AHardwareBuffer_allocate')
        .clear_symbol_version('AHardwareBuffer_describe')
        .clear_symbol_version('AHardwareBuffer_lock')
        .clear_symbol_version('AHardwareBuffer_release')
        .clear_symbol_version('AHardwareBuffer_unlock'),

    'vendor/lib64/anc.hal.so': blob_fixup()
        .add_needed('libion.so'),

    'vendor/bin/qseecom_sample_client': blob_fixup()
        .add_needed('libion.so'),

    (
        'vendor/lib64/libqcrilNr.so', 
        'vendor/lib64/libril-db.so'
    ): blob_fixup()
        .binary_regex_replace(rb'persist\.vendor\.radio\.poweron_opt', b'persist.vendor.radio.poweron_ign'),

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
