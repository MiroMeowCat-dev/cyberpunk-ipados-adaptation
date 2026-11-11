# 来源与许可

| 组件 | 固定来源 | 许可 |
| --- | --- | --- |
| macos-on-ios兼容层 | https://github.com/zorrobyte/macos-on-ios-public ，commit c2562a333d4de4c08da4ceb4a4f1485bddb84e04 | MIT，Ross Fisher，见licenses |
| MacNativeGames触控参考 | https://github.com/ly05010419/MacNativeGames ，commit edb17ab98543c6cfcbc473928f6783741d9452d5 | MIT，见licenses |
| 本地塞尔达触控编辑器/体感数学 | https://github.com/MiroMeowCat-dev/botw-ipados-adaptation | MIT |
| iLoader | https://github.com/nab138/iloader ，commit9797ecaf6ccdcfc68bd2ef4e0b1123910dd847b6 | MIT及独立品牌声明 |
| isideload0.5.2 | https://github.com/nab138/isideload | MIT |
| apple-codesign-quick0.1.0 | https://github.com/Dadoum/apple-crates | LGPL-2.1-or-later |

顶层MIT适用于本仓库原创适配和文档，上游代码保留其归属，LGPL组件保留自己的许可证。签名组件对应修改源码以ZIP提供，全文许可证一并保存。

CD PROJEKT RED及Cyberpunk2077商标、游戏可执行文件、游戏图标/图片/音频、资产、DLC和着色器不属于本项目许可。Apple SDK、操作系统、个人签名身份和资源备份由用户自行保留。

官方技术参考

- https://developer.apple.com/videos/play/wwdc2026/356/
- https://developer.apple.com/videos/play/wwdc2022/10113/
- https://developer.apple.com/documentation/metal/mtlfeatureset
- https://developer.apple.com/documentation/uikit/uiscreen/potentialedrheadroom

代码中的HDR固定调用位置仅适用于manifest固定的游戏执行文件。此判断适配不是跨版本、跨游戏的GPU功能集通用实现。
