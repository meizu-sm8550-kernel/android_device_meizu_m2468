# Meizu 21 Note (M2468) · LineageOS 23.2

M2468 的 LineageOS 23.2 设备配置，已改为源码内核、DT 与配套模块构建。

本仓库由 [kernel_manifest](https://github.com/meizu-sm8550-kernel/kernel_manifest) 一并拉取，分支 `lineage-23.2`。请使用其中的固定 manifest 和构建说明；不需要另套本地接入补丁。

上游：[来源](https://cnb.cool/AstralSpun/android_device_meizu_m2468)，基线提交 `b82127689b3f7aa7a09c51b3af8b8a3140eeaa17`。保留原有许可证及版权声明。这里的 ROM 基准是 LineageOS 23.2；SoC内核基准仍是 Android 13 / Linux 5.15，二者不是同一版本号。

本地已完成核心、385个模块及六份M2468 DT的构建/静态验证，基础模块CRC配套；尚未完成整套ROM构建或真机启动验证。不得混用stock ko、伪造CRC/vermagic或关闭模块检查。

M2468局部HBM目前限部分亮屏模式；AOD、完整ready、手势/指纹、充电扩展与其它设备运行行为仍待验证。源码存在或编译通过不代表这些功能可用。
