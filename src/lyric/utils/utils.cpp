//
// Created by LEGION on 2025/12/10.
//

#include "utils.hpp"
#include <QLatin1StringView>
#include <QRegularExpression>

#include "../lyric_line/lyricline.hpp"

using Qt::Literals::StringLiterals::operator""_L1;

QRegularExpression before_reg(R"(^[\(（]?)");
QRegularExpression after_reg(R"([）\)]?$)");

utils::OpenCCConverter::OpenCCConverter(const BuiltinConfig config): _handle(nullptr), _is_valid(false) {
    // 1. 创建 OpenCC 实例
    const OpenCCResult result = opencc_create(config, &_handle);
    if (result == OpenCCResult::Success && _handle != nullptr) {
        _is_valid = true;
    } else {
        qWarning() << "Failed to create OpenCC instance. Error code:" << static_cast<int>(result);
    }
}

utils::OpenCCConverter::~OpenCCConverter() {
    if (_handle != nullptr) {
        opencc_destroy(_handle);
    }
}

QString utils::OpenCCConverter::convert(const QString &input_text) const {
    if (!_is_valid) {
        qWarning() << "OpenCCConverter is not valid, returning original text.";
        return input_text;
    }

    // 将 QString 转换为 UTF-8 编码的 C 字符串
    const QByteArray text_bytes = input_text.toUtf8();

    // 2. 调用 FFI 函数进行转换
    // ReSharper disable once CppTooWideScope
    char *converted_text = opencc_convert(_handle, text_bytes.constData());

    if (converted_text) {
        // 从返回的 C 字符串创建 QString
        const QString result = QString::fromUtf8(converted_text);
        // 3. 释放 FFI 函数分配的字符串内存
        opencc_free_string(converted_text);
        return result;
    } else {
        qWarning() << "OpenCC conversion failed, returning original text.";
        return input_text; // 转换失败则返回原文
    }
}

bool utils::OpenCCConverter::isValid() const {
    return _is_valid;
}

QString utils::toHtmlEscaped(const QString &text) {
    const auto pos = std::u16string_view(text).find_first_of(u"<>&\"'");
    if (pos == std::u16string_view::npos)
        return text;
    QString rich;
    const qsizetype len = text.size();
    rich.reserve(static_cast<qsizetype>(len * 1.1));
    rich += qToStringViewIgnoringNull(text).first(pos);
    for (auto ch : qToStringViewIgnoringNull(text).sliced(pos)) {
        if (ch == u'<')
            rich += "&lt;"_L1;
        elif (ch == u'>')
            rich += "&gt;"_L1;
        elif (ch == u'&')
            rich += "&amp;"_L1;
        elif (ch == u'"')
            rich += "&quot;"_L1;
        elif (ch == u'\'')
            rich += "&apos;"_L1;
        else
            rich += ch;
    }
    rich.squeeze();
    return rich;
}

QString utils::normalizeBrackets(QString &text) {
    // replace syls.front: _before_reg->'('
    const auto front_match = before_reg.match(text);
    if (front_match.hasMatch()) text = text.replace(front_match.capturedStart(), front_match.capturedLength(), "");
    // replace syls.back: _after_reg->')'
    const auto back_match = after_reg.match(text);
    if (back_match.hasMatch()) text = text.replace(back_match.capturedStart(), back_match.capturedLength(), "");

    return text;
}

LyricLine utils::normalizeBrackets(LyricLine &line) {
    // replace syls.front: _before_reg->'('
    const auto front_match = before_reg.match(line._syl_s.first()->getText());
    if (front_match.hasMatch()) {
        line._syl_s.first()->setText(line._syl_s.first()->getText().replace(front_match.capturedStart(), front_match.capturedLength(), ""));
    }
    // replace syls.back: _after_reg->')'
    const auto back_match = after_reg.match(line._syl_s.last()->getText());
    if (back_match.hasMatch()) {
        line._syl_s.last()->setText(line._syl_s.last()->getText().replace(back_match.capturedStart(), back_match.capturedLength(), ""));
    }

    return line;
}
