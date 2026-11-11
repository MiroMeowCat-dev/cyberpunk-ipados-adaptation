# 构建、恢复与覆盖更新

## 保留的私有输入

恢复需要自己的兼容无签名Documents/GameData型转换IPA、Mac2.5.0游戏资源和存档备份。归档工具只接受清单固定的主执行文件SHA256，并保留原有应用标识。HDR查询调用偏移只适用于该执行文件。

这份档案保存的是已转换宿主的后续适配层。若从全新Mac原包开始，先按固定上游macos-on-ios的转换流程生成基础宿主，并处理本项目Windows工具链、地址空间和Background Assets扩展要求，再核对主程序hash。用户需要自行提供游戏和有权使用的Apple SDK。

## 检查并应用实测模块

在归档目录运行

```powershell
python tools/verify_archive.py
python tools/apply_update.py --input "./private-input/compatible-unsigned.ipa" --output "./private-output/cyberpunk-v21-unsigned.ipa"
```

工具保留私有输入中的游戏内容，只替换清单中的两份兼容库、父子包版本和HDR查询配置。游戏图标沿用私有输入。输出仍需用自己的开发身份签名，父包和Downloader.appex需要各自正确profile及真实App Group。

先保存并退出游戏、备份存档/偏好，再以同一账号和应用标识覆盖安装。不要为了更新先卸载原游戏，因为资源和存档位于其容器。资源导入清单与恢复备份应私下保存。

## 从源码重新编译模块

Windows需要ARM64-iOS Clang/LLD、Python3.11+和本地iPhoneOS26.5 SDK。

```powershell
python tools/build_modules.py --toolchain-bin "./local-toolchain/bin" --sdk "./local-sdk/iPhoneOS26.5.sdk" --out "./private-output/rebuilt-v21"
```

该入口编译公开src中的完整兼容模块，保留UIKit/CoreGraphics reexport。重编译结果是新的测试产物，重新检查导入/导出和真机后才替代实测模块。当前apply_update默认使用update-kit-v21内有hash约束的实测模块。

个人profile、签名密钥、Apple账号、应用容器、USB配对文件及私有工具检查点均不进入公共仓库。
