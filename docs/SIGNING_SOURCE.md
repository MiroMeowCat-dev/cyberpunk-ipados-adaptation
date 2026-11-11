# 签名组件源码

packages/iloader-extensionkit-signing-source.zip包含此次Windows本地测试版iLoader及其两个Rust签名依赖的对应修改源码。

基线iLoader2.4.1，commit9797ecaf6ccdcfc68bd2ef4e0b1123910dd847b6。主要修改涉及发现Extensions目录、对子扩展单独签名和配置profile、同步真实App Group、仅签名检查点，以及将安装放到独立检查之后。账号认证沿用上游实现。

isideload0.5.2采用MIT，apple-codesign-quick0.1.0采用LGPL-2.1-or-later。ZIP包含相应源码和许可证。本档案没有分发修改版签名EXE；构建依赖由使用者自行准备。标识为非官方教育/研究fork，iLoader品牌按上游单独声明使用。

两个公共Apple CA根证书由ZIP里的fetch-public-roots.py从Apple官方地址下载并校验固定hash。个人开发者证书、密钥和profile没有包含在ZIP。签名过程会向Apple申请真实配置，用户需自行输入账号和验证码。

本地签名审计使用OpenSSL -noverify校验CMS签名数学与内容一致性，未验证Apple信任链和吊销状态。最终信任由iPad安装服务判断。原项目曾用合成身份测试篡改拒绝、错设备、错误父子profile和权限等边界。

源码包提供签名逻辑保存和复核，完整桌面构建仍取决于Rust/Node/WebView和系统依赖。请依据源码包内THIRD_PARTY.md恢复环境。
