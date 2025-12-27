//
// Created by LEGION on 2025/12/10.
//

#include "utils.h"
#include <QLatin1StringView>
#include <QRegularExpression>

#include "LyricLine.h"

using Qt::Literals::StringLiterals::operator""_L1;

QRegularExpression before_reg(R"(^[\(（]?)");
QRegularExpression after_reg(R"([）\)]?$)");

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

void utils::trim(LyricLine &line) {
    while (line._syl_s.count()) {
        const auto syl = *line._syl_s.first();
        if (syl.getText().trimmed().isEmpty()) line._syl_s.pop_front();
        else break;
    }
    while (line._syl_s.count()) {
        const auto syl = *line._syl_s.last();
        if (syl.getText().trimmed().isEmpty()) line._syl_s.pop_back();
        else break;
    }
}
