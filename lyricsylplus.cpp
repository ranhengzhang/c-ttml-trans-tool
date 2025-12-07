//
// Created by LEGION on 2025/11/21.
//

#include "lyricsylplus.h"

LyricSylPlus LyricSylPlus::parse(const QDomNode &span) {
    LyricSylPlus syl;

    syl._is_text = span.isText();
    if (span.isText()) {
        syl._text = span.nodeValue();
    } else {
        const auto el = span.toElement();

        const auto begin = LyricTimePlus::parse(el.attribute("begin"));
        syl._begin = begin.first;
        const auto end = LyricTimePlus::parse(el.attribute("end"));
        syl._end = end.first;
        syl._text = el.text();
    }

    return syl;
}

LyricSylPlus LyricSylPlus::parse(const QString &text) {
    LyricSylPlus syl;
    syl._is_text = true;
    syl._text = text;
    return syl;
}

LyricTimePlus LyricSylPlus::getDuration() const {
    return _end - _begin;
}

QString LyricSylPlus::getText() const {
    return _text;
}

void LyricSylPlus::setText(const QString &text) {
    this->_text = text;
}

bool LyricSylPlus::getIsText() const {
    return _is_text;
}

bool LyricSylPlus::isText() const {
    // 先获取 shared_ptr
    auto origPtr = this->_orig.lock();

    return origPtr ? origPtr->isText() : (this->_is_text || ((this->_end - this->_begin) < 5) || this->_text.trimmed().length() == 0);
    // return this->_is_text ||
    //        ((this->_end - this->_begin) < 5) ||
    //        (origPtr && origPtr->isText()) || (!origPtr && this->_text.trimmed().length() == 0);
}

void LyricSylPlus::setIsText(const bool is_text) {
    this->_is_text = is_text;
}

LyricTimePlus LyricSylPlus::getBegin() const {
    return _begin;
}

LyricTimePlus LyricSylPlus::getEnd() const {
    return _end;
}

void LyricSylPlus::setOrig(const std::shared_ptr<LyricSylPlus>& orig_shared) {
    this->_orig = orig_shared;
}

std::shared_ptr<LyricSylPlus> LyricSylPlus::getOrig() const {
    return _orig.lock();
}

QString LyricSylPlus::toTTML(const bool xmlns) {
    return this->isText() ? this->_text : QString(R"(<span begin="%1" end="%2"%3>%4</span>)")
    .arg(this->_begin.toString(false,false,true))
    .arg(this->_end.toString(false,false,true))
    .arg(xmlns ? R"( xmlns="http://www.w3.org/ns/ttml")" : "")
    .arg(this->_text.toHtmlEscaped()) ;
}
