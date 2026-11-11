# 签名组件

`packages/iloader-extensionkit-signing-source.zip` 提供基于 iLoader 2.4.1 的非官方扩展感知签名源码，以及对应修改的 Rust 依赖。仓库不提供预编译签名 EXE。

## 作用

- 发现 `Extensions/Downloader.appex`，为主 App 和子扩展分别配置和使用真实描述文件。
- 同步实际分配的 App Group，保持 Background Assets 配置一致。
- 完成嵌套签名，保留所需内存权限。
- 仅签名并保存检查点，安装由使用者在检查后单独进行。

账号认证沿用固定上游实现，账号、密码和验证码由使用者自行输入。签名过程需要连接 Apple 服务。

## 构建与许可

iLoader 基线 commit 为 `9797ecaf6ccdcfc68bd2ef4e0b1123910dd847b6`。完整构建需要 Rust、Node、WebView 及平台依赖，请按源码包内 `THIRD_PARTY.md` 准备。

`isideload 0.5.2` 使用 MIT；`apple-codesign-quick 0.1.0` 使用 LGPL-2.1-or-later，源码和许可证随包提供。两个公共 Apple CA 根证书由 `fetch-public-roots.py` 从 Apple 官方地址下载并核验哈希。包内没有个人开发者证书、私钥或描述文件。

本地 CMS 检查用于校验签名和内容一致性，最终安装信任由 iPad 安装服务决定。请保留上游归属及品牌声明，不将本地修改版表述为 iLoader 官方发行版。
