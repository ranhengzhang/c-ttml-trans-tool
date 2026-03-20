//
// Created by LEGION on 2025/12/23.
//
#include <ranges>

#include "lyricline.hpp"
#include "lyricsyl.hpp"

int LyricLine::_spl_offset{0};

QString LyricLine::toSingleSPL() {
    QStringList line{};
    auto last = this->getInnerBegin();

    line.push_back(QString(R"([%1])").arg((this->getInnerBegin() + _spl_offset++).toString(true, false, true)));
    for (const auto &syl: this->_syl_s) {
        if (syl->getBegin() > last) {
            line.push_back(QString("<%1>\u200B").arg(last.toString(true, false, true)));
        }
        line.push_back(syl->toSPL());
        if (not syl->isText()) last = syl->getEnd();
    }
    line.push_back(QString(R"([%1])").arg(this->getInnerEnd().toString(true, false, true)));

    auto line_text = line.join("");

    if (this->_is_bg) {
        line_text.insert(line_text.indexOf('>') + 1, '(');
        line_text.insert(line_text.lastIndexOf('['), ')');
    }

    return line_text;
}

QString LyricLine::toSPL() {
    QStringList text{};
    QStringList line{};
    QStringList word{};

    _spl_offset = 0;
    text.push_back(this->toSingleSPL());
    for (const auto &ptr: this->_transliteration | std::views::values) {
        if (const auto roma_pair = std::get_if<std::pair<QString, std::shared_ptr<QString>>>(ptr.get())) {
            if (not roma_pair->first.isEmpty()) line.push_back(roma_pair->first);
        } elif (const auto roma_line = std::get_if<LyricLine>(ptr.get())) {
            word.push_back(roma_line->toSingleSPL());
        } elif (const auto roma_str = std::get_if<QString>(ptr.get())) {
            line.push_back(*roma_str);
        }
    }

    for (const auto &ptr: this->_translation | std::views::values) {
        if (const auto trans_pair = std::get_if<std::pair<QString, std::shared_ptr<QString>>>(ptr.get())) {
            if (not trans_pair->first.isEmpty()) line.push_back(trans_pair->first);
        } elif (const auto trans_line = std::get_if<LyricLine>(ptr.get())) {
            word.push_back(trans_line->toSingleSPL());
        } elif (const auto trans_str = std::get_if<QString>(ptr.get())) {
            line.push_back(*trans_str);
        }
    }

    text.append(word);
    text.append(line);

    if (this->_bg_line) text.append(this->_bg_line->toSPL());

    return text.join("\n");
}
