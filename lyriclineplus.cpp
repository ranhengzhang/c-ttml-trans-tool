//
// Created by LEGION on 2025/11/21.
//

#include "lyriclineplus.h"
#include "lyricsylplus.h"

std::pair<LyricLinePlus, bool> LyricLinePlus::parse(const QDomElement &p, const LyricLinePlus *parent) {
    LyricLinePlus line;

    line._is_bg = !!parent;
    line._is_duet = parent ? parent->_is_duet : "v1" != p.attribute("ttm:agent");
    if (parent == nullptr && p.tagName() == "p") {
        const auto begin = LyricTimePlus::parse(p.attribute("begin"));
        if (!begin.second) return {{}, false};
        line._begin = begin.first;
        const auto end = LyricTimePlus::parse(p.attribute("end"));
        if (!end.second) return {{}, false};
        line._end = end.first;
        line._key = p.attribute(p.tagName() == "text" ? "for" : "itunes:key");
    }

    const auto span_s = p.childNodes();
    for (int i = 0; i < span_s.count(); ++i) {
        auto span = span_s.at(i);
        if (span.toElement().hasAttribute("ttm:role")) {
            auto [bg_line, success] = LyricLinePlus::parse(span.toElement(), &line);
            if (!success) return {{}, false};
            line._bg_line = std::make_shared<LyricLinePlus>(bg_line);
        } else {
            auto syl = LyricSylPlus::parse(span);
            line._syl_s.push_back(std::make_shared<LyricSylPlus>(std::move(syl)));
        }
    }

    return {line, true};
}

void LyricLinePlus::match(const LyricLinePlus &orig) {
    int i = 0;
    int j = 0;

    do {
        while (i < this->_syl_s.size() && this->_syl_s.at(i)->getIsText()) ++i;
        while (j < orig._syl_s.size() && orig._syl_s.at(j)->getIsText()) ++j;

        if (i < this->_syl_s.size() && j < orig._syl_s.size())
            this->_syl_s.at(i)->setOrig(orig._syl_s.at(j));

        ++i;
        ++j;
    } while (i < this->_syl_s.size() && j < orig._syl_s.size());
    for (const auto &syl:this->_syl_s) {
        if (!syl->getOrig()) syl->setIsText(true);
    }

    for (int i = this->_syl_s.length() - 1; i >= 0; --i) {
        const auto &syl = this->_syl_s.at(i);
        if (!syl->isText() && syl->getText().length() > 1 && syl->getText().endsWith(" ")) {
            auto text = syl->getText();
            text.chop(1);
            syl->setText(text);
            this->_syl_s.insert(i + 1, std::make_shared<LyricSylPlus>(LyricSylPlus::parse(" ")));
        }
    }

    if (this->_bg_line && orig._bg_line) this->_bg_line->match(*orig._bg_line);
}

QString LyricLinePlus::toTTML() const {
    return QString(R"(<p begin="%1" end="%2" itunes:key="%3" ttm:agent="v%4">%5</p>)")
    .arg(this->getBegin().toString(false, false, true))
    .arg(this->getEnd().toString(false, false, true))
    .arg(this->_key)
    .arg(this->_is_duet ? "2" : "1")
    .arg(this->toInnerTTML());
}

QString LyricLinePlus::toInnerTTML(const bool xmlns) const {
    QString ret;
    for (const auto &syl : this->_syl_s) ret += syl->toTTML(xmlns);
    if (this->_bg_line)
        ret += QString(R"( <span ttm:role="x-bg"%1>%2</span>)")
        .arg(xmlns ? R"( xmlns:ttm="http://www.w3.org/ns/ttml#metadata" xmlns="http://www.w3.org/ns/ttml")" : "")
        .arg(this->_bg_line->toInnerTTML());
    return ret;
}

LyricTimePlus LyricLinePlus::getBegin() const {
    if (this->_is_bg) {
        if (this->_syl_s.length() > 1) {
            return this->_syl_s.first()->isText() ?
            this->_syl_s.at(1)->getBegin() :
            this->_syl_s.first()->getBegin();
        } else {
            return this->_syl_s.first()->isText() ? LyricTimePlus::parse("9:59:59:999").first : this->_syl_s.first()->getBegin();
        }
    }
    return this->_bg_line ? std::min(this->_bg_line->getBegin(), this->_begin) : this->_begin;
}

LyricTimePlus LyricLinePlus::getEnd() const {
    if (this->_is_bg) {
        if (this->_syl_s.length() > 1) {
            return this->_syl_s.last()->isText() ?
            this->_syl_s.at(this->_syl_s.length() - 2)->getEnd() :
            this->_syl_s.last()->getEnd();
        } else {
            return this->_syl_s.first()->isText() ? LyricTimePlus::parse("0:000").first : this->_syl_s.first()->getEnd();
        }
    }
    return this->_bg_line ? std::max(this->_bg_line->getEnd(), this->_end) : this->_end;
}
