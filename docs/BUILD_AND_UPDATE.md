# 从源码构建

直接使用已提供模块时，按[安装与使用](USAGE.md)操作即可。只有修改兼容层或输入功能时才需要自行编译。

## 环境

Windows、Python 3.11+、ARM64-iOS Clang/LLD，以及有权使用的 iPhoneOS26.5 SDK。编译器、SDK 和签名身份由使用者自行准备。

在仓库目录执行，输出放到仓库以外的新目录：

```powershell
python tools/verify_archive.py
python tools/build_modules.py --toolchain-bin "../local-toolchain/bin" --sdk "../local-sdk/iPhoneOS26.5.sdk" --out "../private-output/rebuilt-v21"
```

该命令编译两份兼容库，保留 UIKit/CoreGraphics 的重导出。新编译结果属于新的构建，需要检查符号和依赖，再进行实际设备测试。

`tools/apply_update.py` 默认应用 `update-kit-v21/` 中有哈希约束的已提供模块，不会自动应用上面新编译的文件。自定义模块应在独立的私有构建流程中替换、更新校验并验证，保留现成模块用于恢复。

HDR 查询兼容受 `update-kit-v21/manifest.json` 内的主程序哈希和调用位置约束，不能把旧位置直接用于其他游戏版本。任何新宿主均需要重新检查。
