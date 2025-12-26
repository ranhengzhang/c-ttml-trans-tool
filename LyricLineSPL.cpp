//
// Created by LEGION on 2025/12/23.
//
#include <ranges>

#include "LyricLine.h"

QString LyricLine::toSPL() {
    QStringList line{};
    auto last = this->_begin;

    line.push_back(QString(R"([%1])").arg(this->_begin.toString(true, false, true)));
    for (const auto &syl: this->_syl_s) {
        if (syl->getBegin() > last) {
            line.push_back(QString(R"(<%1>)").arg(last.toString(true, false, true)));
        }
        line.push_back(syl->toSPL());
        if (not syl->isText()) last = syl->getEnd();
    }
    line.push_back(QString(R"(<%1>)").arg(last.toString(true, false, true)));
    line.push_back(QString(R"([%1])").arg(this->_end.toString(true, false, true)));

    QStringList text {};

    text.push_back(line.join(""));
    if (this->_bg_line) text.push_back(QString("(%1)").arg(this->_bg_line->toTXT()));
    for (const auto &ptr: this->_translation | std::views::values) {
        if (const auto trans_pair = std::get_if<std::pair<QString, std::shared_ptr<QString>>>(ptr.get())) {
            if (not trans_pair->first.isEmpty()) text.push_back(trans_pair->first);
            if (trans_pair->second) text.push_back(QString("(%1)").arg(*trans_pair->second));
        } elif (const auto trans_line = std::get_if<LyricLine>(ptr.get())) {
            text.push_back(trans_line->toTXT());
            if (trans_line->_bg_line) text.push_back(QString("(%1)").arg(trans_line->_bg_line->toTXT()));
        }
    }

    for (const auto &ptr: this->_transliteration | std::views::values) {
        if (const auto roma_pair = std::get_if<std::pair<QString, std::shared_ptr<QString>>>(ptr.get())) {
            if (not roma_pair->first.isEmpty()) text.push_back(roma_pair->first);
            if (roma_pair->second) text.push_back(QString("(%1)").arg(*roma_pair->second));
        } elif (const auto roma_line = std::get_if<LyricLine>(ptr.get())) {
            text.push_back(roma_line->toTXT());
            if (roma_line->_bg_line) text.push_back(QString("(%1)").arg(roma_line->_bg_line->toTXT()));
        }
    }

    return text.join("\n");
}
