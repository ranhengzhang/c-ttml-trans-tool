//
// Created by LEGION on 25-4-27.
//

#include "lyricsyl.h"

#include <QRegularExpression>

QRegularExpression LyricSyl::_before(R"(^\(+)");
QRegularExpression LyricSyl::_after(R"(\)+$)");

LyricSyl LyricSyl::parse(const QDomElement &span, bool *ok) {
    LyricSyl syl;

    syl._text = span.text();
    syl._begin = LyricTime::parse(span.attribute(R"(begin)"), ok);
    if (!ok) return {};
    syl._end = LyricTime::parse(span.attribute(R"(end)"), ok);
    // ReSharper disable once CppDFAConstantConditions
    // ReSharper disable once CppDFAUnreachableCode
    if (!ok) return {};

    *ok = true;
    return syl;
}

LyricSyl LyricSyl::parse(const QString &text, const LyricTime begin, const LyricTime end, bool *ok) {
    if (text.isEmpty()) {
        *ok = false;
        return {};
    }

    LyricSyl syl;

    syl._text = text;
    syl._begin = begin;
    syl._end = end;

    *ok = true;
    return syl;
}

void LyricSyl::setBegin(const LyricTime begin) {
    this->_begin = begin;
}

void LyricSyl::setEnd(const LyricTime end) {
    this->_end = end;
}

LyricTime LyricSyl::getBegin() const {
    return this->_begin;
}

LyricTime LyricSyl::getEnd() const {
    return this->_end;
}

QString LyricSyl::getText() const {
    return this->_text;
}

void LyricSyl::normalize(const NormalizeOption opt) {
    if (NormalizeOption::FRONT == opt) {
        // ReSharper disable once CppTooWideScopeInitStatement
        const auto match = _before.match(this->_text);
        if (match.hasMatch())
            this->_text = this->_text.replace(match.capturedStart(), match.capturedLength(), "");
    } else {
        // ReSharper disable once CppTooWideScopeInitStatement
        const auto match = _after.match(this->_text);
        if (match.hasMatch())
            this->_text = this->_text.replace(match.capturedStart(), match.capturedLength(), "");
    }
}

QString LyricSyl::toTTML() const {
    return this->_begin == this->_end
               ? this->_text
               : QString(R"(<span begin="%1" end="%2">%3</span>)")
               .arg(this->_begin.toString(false, false, true))
               .arg(this->_end.toString(false, false, true))
               .arg(this->_text.toHtmlEscaped());
}

QString LyricSyl::toASS() const {
    auto text = this->_text;

    return QString(R"({\k%1}%2)")
            .arg(std::max(this->_end - this->_begin, 0LL) / 10)
            .arg(this->_text);
}

QString LyricSyl::toSPL() const {
    return QString(R"(<%1>%2)")
            .arg(this->_begin.toString(false, false, true))
            .arg(this->_text);
}

QString LyricSyl::toQRC() const {
    return QString(R"(%1(%2,%3))")
            .arg(this->_text)
            .arg(this->_begin.getCount())
            .arg(std::max(this->_end - this->_begin, 0LL));
}

QString LyricSyl::toYRC() const {
    return QString(R"((%2,%3,0)%1)")
            .arg(this->_text)
            .arg(this->_begin.getCount())
            .arg(std::max(this->_end - this->_begin, 0LL));
}

QString LyricSyl::toKRC(const LyricTime &begin) const {
    return QString(R"(<%1,%2,0>%3)")
            .arg(this->_begin - begin)
            .arg(std::max(this->_end - this->_begin, 0LL))
            .arg(this->_text);
}
