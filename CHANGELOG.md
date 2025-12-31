# CHANGELOG (created by TRAE)

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
>

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