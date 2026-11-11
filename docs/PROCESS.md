# 流程与软件

| 阶段 | 使用的软件与作用 |
| --- | --- |
| 原包检查 | 7-Zip读取用户Mac归档，Mach-O解析、LLVM工具检查架构、导入、平台和依赖 |
| 桥接基础 | macos-on-ios固定commit，UIKit替代AppKit、Metal兼容、资源路径映射 |
| Windows编译 | 本地ARM64-iOS Clang/LLD工具链与iPhoneOS26.5 SDK |
| 签名 | iLoader 2.4.1本地ExtensionKit修改版、isideload0.5.2、apple-codesign-quick0.1.0 |
| 检查与安装 | Python3.11、pymobiledevice3、OpenSSL与本地签名审计/备份/Upgrade脚本 |
| 资源导入 | 7-Zip分块解压、Python经应用容器分批传入Documents/GameData，记录完成清单 |
| iPad输入 | UIKit单画布、GameController虚拟快照、Core Motion |
| 图形验证 | Metal HUD、应用本地限量日志、实际drawable属性采样 |
| UI参考 | 固定MacNativeGames触控面板与本地塞尔达布局编辑器 |

先运行独立窗口/Metal/触控探针，再验证游戏父子包的启动和资源访问。早期程序壳缺资源时仍会退出，导入88项约79,679,358,214字节资源后进入可玩场景。随后逐步修复输入与分辨率，最后对HDR做调用和实际输出双重核对。

玩家本人在签名工具内输入Apple账号、密码与验证码。GUI仅签名并生成私有检查点，检查真实签名、权限、父子profile和App Group后，通过USB对同标识应用执行Upgrade。既有资源与存档留在应用容器中。免费开发签名存在期限及安装数量限制，具体以实际profile和Apple设备返回为准。

本仓库保留对应签名源码ZIP和流程。Windows驱动、iCloud/Apple服务、Python、Rust/Node、编译器与SDK由用户自行准备，工具版本变化后需要重新检查兼容性。
