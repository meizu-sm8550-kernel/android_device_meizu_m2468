// mz_fp_hbm_daemon.cpp - Meizu optical fingerprint HBM control
//
// Prerequisite: gesture_control must be enabled (0x8300011f) by the
//               standalone gesture init service, so the goodix driver
//               reports FOD press/release as key code 0x272.
//
// Protocol: on FOD press, the prebuilt jv HAL sends 20013 (TouchDown)
//           and polls /sys/kernel/display_drivers/hbm_ready_status
//           (~10ms period) until it reads '1'. This daemon writes "6" to
//           the hbm node on press, the panel driver flips hbm_ready_status
//           to '1', the jv HAL then sends 20015 (HBMReady) on its own and
//           the TA captures a lit frame. On release "7" turns it off.
//
// Gating: the same 0x272 also fires on launcher touches, where lighting
//           the highlight would be wrong. The gate works because the jv
//           HAL only reacts to a FOD press while a fingerprint session is
//           active (it then reads input events and polls hbm_ready_status,
//           advancing its cumulative rchar in /proc/<pid>/io). While idle
//           the HAL reads nothing and rchar stays flat. So: probe the
//           HAL's rchar across a short window right after the press.

#include <fcntl.h>
#include <linux/input.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/system_properties.h>
#include <time.h>
#include <unistd.h>

#include <android/log.h>

#define TAG "MzFpHbm"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

static const char kHbmPath[] = "/sys/kernel/display_drivers/hbm";
static const unsigned short kFodKeyCode = 0x272;
static const char kHbmOn[] = "6";
static const char kHbmOff[] = "7";

// The jv HAL service name (see its .rc), used to fetch its pid.
static const char kHalServiceProp[] = "init.svc_debug_pid.anc_fps";

// How long to watch the HAL's rchar after a press. The HAL's poll loop
// advances rchar by ~10 bytes per 100ms while a session is active.
static const int kProbeMs = 100;

static int write_node(const char* path, const void* buf, size_t len) {
    int fd = open(path, O_WRONLY | O_CLOEXEC);
    if (fd < 0) {
        LOGE("open %s failed", path);
        return -1;
    }
    ssize_t n = write(fd, buf, len);
    close(fd);
    return (n == (ssize_t)len) ? 0 : -1;
}

static int find_fod_input_device() {
    for (int i = 0; i < 64; i++) {
        char path[32];
        snprintf(path, sizeof(path), "/dev/input/event%d", i);
        int fd = open(path, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
        if (fd < 0) continue;
        unsigned char bits[KEY_MAX / 8 + 1] = {};
        if (ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(bits)), &bits) >= 0 &&
            (bits[kFodKeyCode / 8] & (1 << (kFodKeyCode % 8)))) {
            char name[80] = {};
            ioctl(fd, EVIOCGNAME(sizeof(name) - 1), name);
            LOGI("FOD device: %s (%s)", path, name);
            return fd;
        }
        close(fd);
    }
    return -1;
}

static pid_t get_hal_pid() {
    char value[PROP_VALUE_MAX] = {};
    int len = __system_property_get(kHalServiceProp, value);
    if (len <= 0) return -1;
    return atoi(value);
}

static long long read_hal_rchar(pid_t pid) {
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/io", pid);
    FILE* f = fopen(path, "re");
    if (f == NULL) return -1;
    char line[128];
    long long val = -1;
    while (fgets(line, sizeof(line), f) != NULL) {
        if (sscanf(line, "rchar: %lld", &val) == 1) break;
    }
    fclose(f);
    return val;
}

// True if the jv HAL reacted to this press (i.e. a fingerprint session is
// active and its event reader / hbm poll loop is running).
static bool hal_is_responding() {
    pid_t pid = get_hal_pid();
    if (pid <= 0) {
        LOGE("hal pid unavailable");
        return false;
    }
    long long before = read_hal_rchar(pid);
    if (before < 0) {
        LOGE("read /proc/%d/io failed", pid);
        return false;
    }
    usleep(kProbeMs * 1000);
    long long after = read_hal_rchar(pid);
    return after > before;
}

int main() {
    LOGI("daemon start");

    int fd = -1;
    bool hbm_on = false;

    for (;;) {
        if (fd < 0) {
            fd = find_fod_input_device();
            if (fd < 0) {
                sleep(2);
                continue;
            }
        }

        struct pollfd pfd = {.fd = fd, .events = POLLIN};
        int ret = poll(&pfd, 1, 5000);
        if (ret < 0) {
            LOGE("poll failed");
            close(fd);
            fd = -1;
            continue;
        }
        if (ret == 0) continue;

        struct input_event ev;
        while (read(fd, &ev, sizeof(ev)) == sizeof(ev)) {
            if (ev.type != EV_KEY || ev.code != kFodKeyCode) continue;

            if (ev.value == 1 && !hbm_on) {  // press
                if (hal_is_responding()) {
                    if (write_node(kHbmPath, kHbmOn, 1) == 0) {
                        hbm_on = true;
                        LOGI("hbm on");
                    }
                } else {
                    LOGI("press ignored: fingerprint HAL idle");
                }
            } else if (ev.value == 0 && hbm_on) {  // release
                if (write_node(kHbmPath, kHbmOff, 1) == 0) {
                    hbm_on = false;
                    LOGI("hbm off");
                }
            }
        }
    }
}