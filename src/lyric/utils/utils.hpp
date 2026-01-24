//
// Created by LEGION on 2025/12/10.
//

#ifndef TTML_TOOL_UTILS_H
#define TTML_TOOL_UTILS_H

#define elif else if

#include <QString>

#include "../ferrous_opencc/opencc.h"

class LyricLine;

namespace utils {
    /**
     * @brief 附加行对象，挂载到行的 <code>lang</code> 下或者 object 的 <code>lang->key</code> 下
     */
    using LyricTrans = std::variant<std::pair<QString, std::shared_ptr<QString>>, LyricLine, QString>;

    enum class SubType {
        Transliteration, // 音译
        Translation // 翻译
    };

    enum class Status {
        Success, // 成功
        InvalidFormat, // 无法解析为 Dom
        InvalidStructure, // Dom 结构错误
        InvalidTimeFormat // 时间格式错误
    };

    // 一个 C++ 包装类，用于管理 OpenCC 实例的生命周期
    // ReSharper disable once CppInconsistentNaming
    class OpenCCConverter {
    public:
        // 构造函数，传入所需的转换配置
        explicit OpenCCConverter(BuiltinConfig config);

        // 析构函数，自动释放 OpenCC 实例
        ~OpenCCConverter();

        // 执行文本转换
        [[nodiscard]] QString convert(const QString &input_text) const;

        // 检查转换器实例是否成功创建
        [[nodiscard]] bool isValid() const;

        // 禁止拷贝构造和赋值，避免对同一个 C-style handle 的重复管理
        OpenCCConverter(const OpenCCConverter &) = delete;
        OpenCCConverter &operator=(const OpenCCConverter &) = delete;

    private:
        OpenCCHandle *_handle{}; // OpenCC 的不透明句柄
        bool _is_valid;         // 标记句柄是否有效
    };

    QString toHtmlEscaped(const QString &text);

    QString normalizeBrackets(QString &text);

    LyricLine normalizeBrackets(LyricLine &line);
}

#endif //TTML_TOOL_UTILS_H
