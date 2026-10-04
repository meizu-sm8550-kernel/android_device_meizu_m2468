#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

namespace {

    constexpr const char kGesturePath[] = "/sys/devices/platform/mx-gs/gesture_control";
    constexpr uint32_t kGestureMask = 0x8300011f;
    constexpr int kMaxRetry = 10;

    bool write_mask_binary() {
        int fd = TEMP_FAILURE_RETRY(open(kGesturePath, O_WRONLY | O_CLOEXEC));
        if (fd < 0) {
            fprintf(stderr, "open %s failed: %s\n", kGesturePath, strerror(errno));
            return false;
        }

        const uint8_t buf[4] = {
                static_cast<uint8_t>(kGestureMask & 0xff),
                static_cast<uint8_t>((kGestureMask >> 8) & 0xff),
                static_cast<uint8_t>((kGestureMask >> 16) & 0xff),
                static_cast<uint8_t>((kGestureMask >> 24) & 0xff),
        };

        const ssize_t written = TEMP_FAILURE_RETRY(write(fd, buf, sizeof(buf)));
        close(fd);

        if (written != static_cast<ssize_t>(sizeof(buf))) {
            fprintf(stderr, "write %s failed: wrote=%zd errno=%s\n", kGesturePath, written,
                    strerror(errno));
            return false;
        }
        return true;
    }

    bool verify_mask() {
        int fd = TEMP_FAILURE_RETRY(open(kGesturePath, O_RDONLY | O_CLOEXEC));
        if (fd < 0) {
            fprintf(stderr, "open read %s failed: %s\n", kGesturePath, strerror(errno));
            return false;
        }

        char buf[64] = {0};
        const ssize_t n = TEMP_FAILURE_RETRY(read(fd, buf, sizeof(buf) - 1));
        close(fd);
        if (n <= 0) {
            fprintf(stderr, "read %s failed: n=%zd errno=%s\n", kGesturePath, n, strerror(errno));
            return false;
        }

        unsigned int v = 0;
        if (sscanf(buf, "%x", &v) != 1) {
            fprintf(stderr, "parse %s failed, content=%s\n", kGesturePath, buf);
            return false;
        }
        return v == kGestureMask;
    }

}  // namespace

int main() {
    for (int i = 0; i < kMaxRetry; ++i) {
        if (write_mask_binary() && verify_mask()) {
            return 0;
        }
        sleep(1);
    }

    // Do not block boot on this helper.
    return 0;
}