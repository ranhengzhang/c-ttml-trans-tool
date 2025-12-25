//
// Created by LEGION on 2025/12/10.
//

#ifndef TTML_TOOL_UTILS_H
#define TTML_TOOL_UTILS_H

#define elif else if

#include <QString>

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

    QString toHtmlEscaped(const QString &text);

    QString normalizeBrackets(QString &text);

    LyricLine normalizeBrackets(LyricLine &line);

    enum class Status {
        Success, // 成功
        InvalidFormat, // 无法解析为 Dom
        InvalidStructure, // Dom 结构错误
        InvalidTimeFormat // 时间格式错误
    };
}

#endif //TTML_TOOL_UTILS_H
