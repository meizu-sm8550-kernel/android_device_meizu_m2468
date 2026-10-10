# m2468 设备构建配置

M2468 的 Android 设备配置，负责从源码构建内核、外置驱动和 DT，并设置模块加载顺序与指纹 HAL 权限。

本仓库属于 **meizu-sm8550-kernel**，**当前仅支持 m2468（魅族 21 Note）**。组织与内核仓库名称中的 `sm8550` 表示平台，不表示支持其它魅族 SM8550 设备；M2481（魅族 21 Pro）也不在本适配范围内。

发布分支为 `lineage-23.2`。使用 [kernel_manifest](https://github.com/meizu-sm8550-kernel/kernel_manifest) 同步四个配套源码仓库，并按其中的构建说明编译。清单跟随该分支，`revisions.lock.json` 只记录发布版本。

设备路径、配置和自有代码标识使用 `m2468` / `M2468`。提交采用“子系统前缀 + 首字母大写的动作描述”，每条提交聚焦一项修改，见 [提交约定](https://github.com/meizu-sm8550-kernel/kernel_manifest/blob/lineage-23.2/CONTRIBUTING.md)。原厂 DT 属性、固件名和运行时接口保持兼容。

上游基线为 `b82127689b3f7aa7a09c51b3af8b8a3140eeaa17`，来源和许可信息见 [m2468-source-provenance.json](m2468-source-provenance.json)。保留上游许可证和版权声明。

本次整理只调整提交历史、内部命名和文档。此前编译与设备反馈的范围见 [历史适配记录](docs/m2468-bringup-history.md)；未因本次整理新增整 ROM 编译或实机验证结论。各项主机回归不能代替外设运行验证。

`BoardConfig.mk` 集中配置源码内核、模块及 DTBO 构建。OTA 兼容别名 `meizu21Note` 保留；它是既有设备识别接口，不用作新增适配代码的代号。
