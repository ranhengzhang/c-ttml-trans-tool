//
// Created by LEGION on 2025/12/23.
//
#include <ranges>

#include "LyricLine.h"

QString LyricLine::toSPL() {
    QStringList line{};
    auto last = this->_begin;

    line.append(QString(R"([%1])").arg(this->_begin.toString(false, false, true)));
    for (const auto &syl: this->_syl_s) {
        if (syl->getBegin() > last) {
            line.append(QString(R"(<%1>)").arg(last.toString(false, false, true)));
        }
        line.append(syl->toSPL());
        if (not syl->isText()) last = syl->getEnd();
    }
    line.append(QString(R"(<%1>)").arg(last.toString(false, false, true)));
    line.append(QString(R"([%1])").arg(this->_end.toString(false, false, true)));

    QStringList text {};

    text.append(line.join(""));
    if (this->_bg_line) text.append(QString("(%1)").arg(this->_bg_line->toTXT()));
    for (const auto &ptr: this->_translation | std::views::values) {
        if (const auto trans_pair = std::get_if<std::pair<QString, std::shared_ptr<QString>>>(ptr.get())) {
            if (not trans_pair->first.isEmpty()) text.append(trans_pair->first);
            if (trans_pair->second) text.append(QString("(%1)").arg(*trans_pair->second));
        } elif (const auto trans_line = std::get_if<LyricLine>(ptr.get())) {
            text.append(trans_line->toTXT());
            if (trans_line->_bg_line) text.append(QString("(%1)").arg(trans_line->_bg_line->toTXT()));
        }
    }

    for (const auto &ptr: this->_transliteration | std::views::values) {
        if (const auto roma_pair = std::get_if<std::pair<QString, std::shared_ptr<QString>>>(ptr.get())) {
            if (not roma_pair->first.isEmpty()) text.append(roma_pair->first);
            if (roma_pair->second) text.append(QString("(%1)").arg(*roma_pair->second));
        } elif (const auto roma_line = std::get_if<LyricLine>(ptr.get())) {
            text.append(roma_line->toTXT());
            if (roma_line->_bg_line) text.append(QString("(%1)").arg(roma_line->_bg_line->toTXT()));
        }
    }

    return text.join("\n");
}
