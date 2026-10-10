# M2468 历史适配记录

以下保留各轮记录的时间、验证范围和当时状态；当前入口见 [README](../README.md)。其中下游提交号已按 [历史映射](https://github.com/meizu-sm8550-kernel/kernel_manifest/blob/lineage-23.2/history/2026-10-10-commit-map.json) 更新。

2026-10-06 相机供应者接入：启用现有 PM8008 chip/LDO 驱动并显式第二阶段加载，补齐 M2468 原 DT 的相机电源供应者。相机 master 等待这些组件，因而此前未创建 `/sys/kernel/camera/subparts_info`，CamX 在初始化时中止。原 DT、电压和 camera 驱动保持。

本地仅增量编译 PM8008，通过34项导入CRC与CFI/ThinLTO；新选择388项，在既有HBM387上只增加供应者。候选未加载，provider、传感器枚举、预览、拍照及录像均待用户更新镜像后验证。

# Meizu 21 Note (M2468) · lineage-23.2

M2468 设备配置，使用源码内核、配套外置驱动与 M2468 DTS 构建。

通过 [kernel_manifest](https://github.com/meizu-sm8550-kernel/kernel_manifest) 的分支跟随清单同步；不需要人工套补丁。发布分支为 `lineage-23.2`，不继承 ROM 分支，不固定项目 SHA，也不移除其他清单项目。

[公开上游](https://cnb.cool/AstralSpun/android_device_meizu_m2468)，基线 `b82127689b3f7aa7a09c51b3af8b8a3140eeaa17`；保留原有许可证及版权声明。内核基线保持 SM8550 / Kalama / Android13 Linux5.15。ROM 构建规则参考官方 LineageOS23.2，用户运行的是24.0 / Android17，不能称为官方23.2整ROM验证。

既有源码基线已进入系统；用户确认ESD黑闪、bark误按键、Wi-Fi基本使用和启动提速。新的 JIIOV 候选仅完成源码接口回归、配套本地编译/CRC检查与加载配置，尚未加载到设备，也未验证probe、HAL初始化、TEE/校准、HBM、录入、匹配或解锁。

本地原386模块基线保留；选择库存增加 `jiiov_fingerprint`，加上既有bark与WLAN替换，共387项。基础内核和M2468 DT不变，不混用stock ko、不伪造CRC/vermagic、不关闭CFI/MODVERSIONS。

JIIOV保持M2468原DT参数及精确ioctl、netlink30/port100接口。配套M2468构建选择 `CONFIG_WLAN_DISABLE_CESIUM_NETLINK=y`，只释放无收发逻辑的Cesium占位socket；其它WLAN诊断通道保留。新增两模块的实机共存和Wi-Fi回归仍需验证。

指纹节点使用专用SELinux类型和受限ioctl规则；离线当前ROM策略对比无新增neverallow冲突，但基线策略存在两条冲突，不代表完整ROM策略编译通过。

完整显示/AOD、音频播放录音、相机、充电、温控及其它OEM行为仍有未验证内容。源码存在和编译成功不是功能恢复。
