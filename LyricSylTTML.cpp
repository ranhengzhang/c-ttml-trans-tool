//
// Created by LEGION on 2025/12/17.
//

#include "LyricSyl.h"

std::pair<LyricSyl, LyricSyl::Status> LyricSyl::fromTTML(const QDomNode &span) {
    if (span.isText()) {
        return {fromText(span.nodeValue()), Status::Success};
    }

    LyricSyl syl;
    const auto el = span.toElement();

    syl._is_text = false;
    syl._is_explicit = el.hasAttribute("amll:obscene") and el.attribute("amll:obscene") == "true";
    const auto begin = LyricTime::parse(el.attribute("begin"));
    if (!begin.second) return {{}, Status::InvalidTimeFormat};
    syl._begin = begin.first;
    const auto end = LyricTime::parse(el.attribute("end"));
    if (!end.second) return {{}, Status::InvalidTimeFormat};
    syl._end = end.first;
    syl._text = el.text();

    return {syl, Status::Success};
}

QString LyricSyl::toTTML(bool xmlns) {
    return this->isText() ? utils::toHtmlEscaped(this->_text) : QString(R"(<span begin="%1" end="%2"%3%4>%5</span>)")
    .arg(this->_begin.toString(false,false,true))
    .arg(this->_end.toString(false,false,true))
    .arg(this->_is_explicit ? R"( amll:obscene="true")" : "")
    .arg(xmlns ? R"( xmlns="http://www.w3.org/ns/ttml")" : "")
    .arg(utils::toHtmlEscaped(this->_text)) ;
}

void LyricSyl::setOrig(const std::shared_ptr<LyricSyl> &orig_shared) {
    this->_orig = orig_shared;
}

std::shared_ptr<LyricSyl> LyricSyl::getOrig() const {
    return this->_orig.lock();
}
