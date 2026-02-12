# CHANGELOG (created by TRAE)

### 1.6.6

> [!IMPORTANT]
>
> **提交记录 (History)**
>
> - `899bfec` fix(ui): Preserve scroll position when compressing TTML text
> - `f25646a` feat(config): Add user configuration management for Github ID

> [!NOTE]
>
> **功能增强 (Features)**
>
> - 添加用户配置管理功能，使用 QSettings 实现配置的持久化存储
> - 添加 "配置" 菜单，包含 Github ID 配置选项
> - 更新文件名和命令生成以使用用户配置的 ID，替换硬编码 ID
> - 添加 settings.ini 到 .gitignore 以保护用户配置

> [!CAUTION]
>
> **错误修复 (Bug Fixes)**
>
> - 修复压缩 TTML 文本时滚动位置重置的问题
> - 在文本压缩前保存滚动位置百分比
> - 在文本压缩完成后恢复滚动位置
> - 改善用户体验，防止滚动位置意外重置

### 1.6.4

> [!IMPORTANT]
>
> **提交记录 (History)**
>
> - `daa3c2e` feat(app): Add single instance detection and IPC functionality
> - `a07f1e3` refactor(project): Massive project restructuring and build system improvements
> - `2dc6f2c` refactor(project): Comprehensive codebase restructuring and modernization
> - `9d495ab` feat(lyric): Add songwriter extraction and display functionality

> [!NOTE]
>
> **功能增强 (Features)**
>
> - 添加 "作词" 菜单项到主窗口，支持复制词曲作者为 Markdown 表格
> - 为 LyricObject 类添加 getSongWriter() 方法
> - 实现单实例检测功能，防止重复启动程序
> - 添加 IPC 通信功能，通过 QLocalServer 实现进程间数据传输
> - 添加 "复制生成命令" 菜单项，生成 PowerShell 命令用于写入到 db 文件
> - 为菜单项添加工具提示，提高用户体验
> - 改进 UI 枚举使用，使用显式的 Qt::ScrollBarPolicy 命名空间

> [!TIP]
>
> **代码重构 (Refactoring)**
>
> - 全面重构项目结构，优化模块边界和代码组织
> - 将 OpenCCConverter 改为使用 std::unique_ptr 进行内存管理
> - 修复头文件包含路径和前向声明
> - 改进代码风格，使用一致的参数命名
> - 更正 widgets 模块中的 CMake 文件变量名称
> - 现代化项目结构，使用更清晰的模块边界
> - 移除未使用的 Network 组件依赖

> [!CAUTION]
>
> **构建修复 (Build Fixes)**
>
> - 添加 src/dialogs/CMakeLists.txt 用于模块化对话框构建
> - 更新 CMakeLists.txt 使用 dialogs 子模块代替直接源文件
> - 修复版本资源中调试构建的错误 DLL 标志
> - 确保正确的 Windows 文件属性生成

### 1.4.2

> [!IMPORTANT]
>
> **提交记录 (History)**
>
> - `2468d47` fix(build): Remove incorrect DLL flag from version resource
> - `1bbbd80` fix(lyricline): Prevent null pointer dereference in TTML translation parsing

> [!CAUTION]
>
> **错误修复 (Bug Fixes)**
>
> - 修复 TTML 翻译解析中的空指针解引用问题，添加空指针检查
> - 初始化空的翻译/音译指针，防止解析背景行时崩溃
> - 确保翻译结构的正确初始化

> [!NOTE]
>
> **构建修复 (Build Fixes)**
>
> - 移除版本资源中调试构建的错误 DLL 标志(VS_FF_DLL)
> - 修复应用程序构建类型的版本资源配置
> - 确保正确的 Windows 文件属性生成

## 1.4

> [!IMPORTANT]
>
> **提交记录 (History)**
>
> - `8123bb4` feat(lyricline): improve time calculation and TTML output for lyric lines

> [!NOTE]
>
> **功能增强 (Features)**
>
> - 增强 getInnerBegin() 和 getInnerEnd() 方法以处理纯文本音节
> - 修复 TTML 解析中背景行的翻译和音译分配
> - 改进背景行的 TTML 输出格式，使用适当的括号处理
> - 为 lyricline.h 中的方法功能添加中文注释
> - 优化背景行的 TTML 输出结构

> [!TIP]
>
> **代码重构 (Refactoring)**
>
> - 重构 getInnerBegin() 和 getInnerEnd() 方法的实现逻辑

## 1.3

> [!IMPORTANT]
>
> **提交记录 (History)**
>
> - `c7b8710` refactor: improve code quality and error handling

> [!NOTE]
>
> **功能增强 (Features)**
>
> - 实现 selectLang 函数用于更好的语言选择逻辑
> - 为元数据键和值添加 HTML 转义，提高安全性
> - 增强翻译语言排序，优先处理中文

> [!CAUTION]
>
> **错误修复 (Bug Fixes)**
>
> - 改进 mainwindow.cpp 中的错误消息处理，提供更详细的错误信息

> [!TIP]
>
> **代码重构 (Refactoring)**
>
> - 在 LyricLineTTML.cpp 中为 xmlns 参数添加 const 限定符，提高代码质量

## 1.2

> [!IMPORTANT]
>
> **提交记录 (History)**
>
> - `f98a7d0` feat(TTML): support both songPart and song-part attributes for song sections

> [!NOTE]
>
> **功能增强 (Features)**
>
> - 为 TTML 解析添加对 'itunes:songPart' 和 'itunes:song-part' 两种属性的支持
> - 改进歌曲分段检测，先检查属性是否存在
> - 保持与现有 TTML 文件的向后兼容性

## 1.1

> [!IMPORTANT]
>
> **提交记录 (History)**
>
> - `b237eb4` feat: integrate OpenCC library for Chinese conversion
> - `8de2528` fix(TTML): prevent iterator dereference and improve parsing safety

> [!NOTE]
>
> **功能增强 (Features)**
>
> - 集成 OpenCC 库用于中文转换
> - 实现 OpenCCConverter 包装类，确保安全使用
> - 为 opencc.h API 添加详细文档
> - 重组 utils.h，将 Status 枚举移至顶部

> [!CAUTION]
>
> **错误修复 (Bug Fixes)**
>
> - 修复 TTML 解析中的迭代器解引用问题
> - 为缺失的行自动生成键，防止查找失败
> - 转义 TTML 导出中的歌曲作者名称，防止 XML 注入
> - 移除 compressTtmlV2 中未使用的 QDomDocument 变量

## 1.0

> [!IMPORTANT]
>
> **提交记录 (History)**
>
> - `8ec5fa2` feat: Multiple enhancements across lyric processing
> - `adf7b8d` feat: Multiple improvements and bug fixes across lyric formats
> - `117dd08` feat(SPL): Update time formatting to use leading zeros for minutes
> - `8de30f8` feat(LyricLineTTML): Add proper timing for text-type syllables in TTML lines
> - `3c86aaa` refactor: improve code style and naming conventions
> - `f71069a` refactor: replace QStringList::append with push_back for consistency

> [!NOTE]
>
> **功能增强 (Features)**
>
> - 为 SPL 导出添加时间标签后的零宽连接符，提升格式化效果
> - 改进对唱检测方法
> - 增强 LRC 导出选项，支持多种音译语言
> - 更新 TTML 行的 `trim` 功能，移除空音节
> - 为 TTML 文本类型音节添加正确的时间设置
> - 更新 SPL 时间格式，使用分钟前导零
> - 改进 LyricTime 的 min/max 值和算术运算符
> - 更新类接口以提高灵活性

> [!CAUTION]
>
> **错误修复 (Bug Fixes)**
>
> - 修复 ASS 导出问题：重复持续时间、音译/翻译混淆、缺少结束时间
> - 修复 XML 翻译函数，跳过 `head` 标签而非 `metadata` 标签
> - 修复多种导出格式的时间格式化问题


> [!TIP]
>
> **代码重构 (Refactoring)**
>
> - 改进代码风格和命名约定
> - 替换 `QStringList::append` 为 `push_back` ，保持一致性
> - 重命名函数为 `camelCase` 格式（如 `compress_ttml*` -> `compressTtml*`）
> - 重命名变量为 `snake_case` 格式（如 `timeList` -> `time_list`, `durFormat` -> `dur_format`）
> - 更新函数参数使用 `const` 引用（如 `s2t/t2s(QString)` -> `s2t/t2s(const QString&)`）
> - 移除未使用的标签并调整预设元数据处理
> - 仅对非空文本音节应用 `\-T` 标签
> - 将 `LyricLine::toASS` 默认 `role` 参数从 `""` 更新为 `"orig"`