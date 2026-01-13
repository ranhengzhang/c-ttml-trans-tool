//
// Created by LEGION on 2025/12/14.
//

#include "lyricline.hpp"

#include <ranges>

#include <QList>

QString LyricLine::toTXT() const {
    std::span spanSyls = this->_syl_s;
    auto resultView = spanSyls
                    | std::views::transform([](const std::shared_ptr<LyricSyl>& syl){
                        return syl->getText();
                    });
    return QStringList(resultView.begin(), resultView.end()).join("");
}

void LyricLine::appendSubLine(const SubType role, const QString &lang, const std::shared_ptr<LyricTrans> &content) {
    if (role == SubType::Translation) {
        this->_translation[lang] = content;
        if (auto sub_line = std::get_if<LyricLine>(content.get())) {
            sub_line->setBegin(this->_begin);
            sub_line->setEnd(this->_end);
            if (this->_bg_line and sub_line->_bg_line) {
                sub_line->_bg_line->setBegin(this->_bg_line->_begin);
                sub_line->_bg_line->setEnd(this->_bg_line->_end);
                this->_bg_line->_translation[lang] = std::make_shared<LyricTrans>(*sub_line->_bg_line);
            }
        } elif (const auto sub_pair = std::get_if<std::pair<QString, std::shared_ptr<QString>>>(content.get())) {
            if (this->_bg_line and sub_pair->second) {
                this->_bg_line->_translation[lang] = std::make_shared<LyricTrans>(*sub_pair->second);
            }
        }
    } else {
        this->_transliteration[lang] = content;
        if (const auto sub_line = std::get_if<LyricLine>(content.get())) {
            sub_line->setBegin(this->_begin);
            sub_line->setEnd(this->_end);
            if (this->_bg_line and sub_line->_bg_line) {
                sub_line->_bg_line->setBegin(this->_bg_line->_begin);
                sub_line->_bg_line->setEnd(this->_bg_line->_end);
                this->_bg_line->_transliteration[lang] = std::make_shared<LyricTrans>(*sub_line->_bg_line);
            }
        } elif (const auto sub_pair = std::get_if<std::pair<QString, std::shared_ptr<QString>>>(content.get())) {
            if (this->_bg_line and sub_pair->second) {
                this->_bg_line->_transliteration[lang] = std::make_shared<LyricTrans>(*sub_pair->second);
            }
        }
    }
}

void LyricLine::match(const LyricLine &orig) {
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

    for (qsizetype n = this->_syl_s.length() - 1; n >= 0; --n) {
        // ReSharper disable once CppTooWideScopeInitStatement
        const auto &syl = this->_syl_s.at(n);
        if (!syl->isText() && syl->getText().length() > 1 && syl->getText().endsWith(" ")) {
            auto text = syl->getText();
            text.chop(1);
            syl->setText(text);
            this->_syl_s.insert(n + 1, std::make_shared<LyricSyl>(LyricSyl::fromText(" ")));
        }
    }
    this->trim();

    if (this->_bg_line && orig._bg_line) this->_bg_line->match(*orig._bg_line);
}

void LyricLine::trim() {
    while (this->_syl_s.count()) {
        const auto syl = *this->_syl_s.first();
        if (not syl.getOrig() and syl.getText().trimmed().isEmpty()) this->_syl_s.pop_front();
        else break;
    }
    while (this->_syl_s.count()) {
        const auto syl = *this->_syl_s.last();
        if (not syl.getOrig() and syl.getText().trimmed().isEmpty()) this->_syl_s.pop_back();
        else break;
    }
}

LyricTime LyricLine::getBegin() const {
    return this->_begin;
}

void LyricLine::setBegin(const LyricTime &time) {
    this->_begin = time;
}

LyricTime LyricLine::getEnd() const {
    return this->_end;
}

void LyricLine::setEnd(const LyricTime &time) {
    this->_end = time;
}

LyricTime LyricLine::getDuration() const {
    return this->_end - this->_begin;
}

LyricTime LyricLine::getLineBegin() const {
    if (this->_is_bg) {
        if (this->_syl_s.length() > 1) {
            return this->_syl_s.first()->isText() ?
            this->_syl_s.at(1)->getBegin() :
            this->_syl_s.first()->getBegin();
        } else {
            return this->_syl_s.first()->isText() ? LyricTime::parse("9:59:59:999").first : this->_syl_s.first()->getBegin();
        }
    }
    return this->_bg_line ? std::min(this->_bg_line->getLineBegin(), this->_begin) : this->_begin;
}

LyricTime LyricLine::getLineEnd() const {
    if (this->_is_bg) {
        if (this->_syl_s.length() > 1) {
            return this->_syl_s.last()->isText() ?
            this->_syl_s.at(this->_syl_s.length() - 2)->getEnd() :
            this->_syl_s.last()->getEnd();
        } else {
            return this->_syl_s.first()->isText() ? LyricTime::parse("0:000").first : this->_syl_s.first()->getEnd();
        }
    }
    return this->_bg_line ? std::max(this->_bg_line->getLineEnd(), this->_end) : this->_end;
}

LyricTime LyricLine::getLineDuration() const {
    return this->getLineEnd() - this->getLineBegin();
}

LyricTime LyricLine::getInnerBegin() const {
    const auto real_syl_s = std::ranges::find_if(this->_syl_s, [](const auto &syl) { return not syl->isText(); });
    return real_syl_s == this->_syl_s.end() ? this->_begin : std::max(this->_begin, real_syl_s->get()->getBegin());
}

LyricTime LyricLine::getInnerEnd() const {
    auto reversed = std::views::reverse(this->_syl_s);
    const auto real_syl_s = std::ranges::find_if(reversed, [](const auto &syl) { return not syl->isText(); });
    return real_syl_s == reversed.end() ? this->_end : std::min(this->_end, real_syl_s->get()->getEnd());
}

LyricTime LyricLine::getInnerDuration() const {
    return this->getInnerEnd() - this->getInnerBegin();
}

QString LyricLine::getKey() const {
    return this->_key;
}

bool LyricLine::isDuet() const {
    return this->_is_duet;
}

bool LyricLine::haveBgLine() const {
    return this->_bg_line != nullptr;
}

std::shared_ptr<LyricLine> LyricLine::getBgLine() const {
    return this->_bg_line;
}
