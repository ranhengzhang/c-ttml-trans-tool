//
// Created by LEGION on 2025/12/10.
//

#include <QRegularExpression>

#include "utils.hpp"
#include "lyricsyl.hpp"
#include "lyricline.hpp"

using Qt::Literals::StringLiterals::operator""_L1;

QRegularExpression before_reg(R"(^[\(（]?)");
QRegularExpression after_reg(R"([）\)]?$)");

QString lyric::utils::toHtmlEscaped(const QString &text) {
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

QString lyric::utils::normalizeBrackets(QString &text) {
    // replace syls.front: _before_reg->'('
    const auto front_match = before_reg.match(text);
    if (front_match.hasMatch()) text = text.replace(front_match.capturedStart(), front_match.capturedLength(), "");
    // replace syls.back: _after_reg->')'
    const auto back_match = after_reg.match(text);
    if (back_match.hasMatch()) text = text.replace(back_match.capturedStart(), back_match.capturedLength(), "");

    return text;
}

LyricLine lyric::utils::normalizeBrackets(LyricLine &line) {
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

QString lyric::utils::getDeepInnerText(const QDomNode &node) {
    QString result;
    const QDomNodeList children = node.childNodes();

    for (int i = 0; i < children.count(); ++i) {
        QDomNode child = children.at(i);

        if (child.isText() || child.isCDATASection()) {
            // 如果是文本节点，直接取值
            result += child.toText().data();
        } else if (child.isElement()) {
            // 如果是元素节点，递归获取其内部文本
            result += getDeepInnerText(child);
        }
    }
    return result;
}
