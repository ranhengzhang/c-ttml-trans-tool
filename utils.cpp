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
    const auto frontMatch = before_reg.match(text);
    if (frontMatch.hasMatch()) text = text.replace(frontMatch.capturedStart(), frontMatch.capturedLength(), "");
    // replace syls.back: _after_reg->')'
    const auto backMatch = after_reg.match(text);
    if (backMatch.hasMatch()) text = text.replace(backMatch.capturedStart(), backMatch.capturedLength(), "");

    return text;
}

LyricLine utils::normalizeBrackets(LyricLine &line) {
    // replace syls.front: _before_reg->'('
    const auto frontMatch = before_reg.match(line._syl_s.first()->getText());
    if (frontMatch.hasMatch()) {
        line._syl_s.first()->setText(line._syl_s.first()->getText().replace(frontMatch.capturedStart(), frontMatch.capturedLength(), ""));
    }
    // replace syls.back: _after_reg->')'
    const auto backMatch = after_reg.match(line._syl_s.last()->getText());
    if (backMatch.hasMatch()) {
        line._syl_s.last()->setText(line._syl_s.last()->getText().replace(backMatch.capturedStart(), backMatch.capturedLength(), ""));
    }

    return line;
}
