//
// Created by LEGION on 2025/12/23.
//
#include <ranges>

#include "LyricLine.h"

QString LyricLine::toSPL() {
    QStringList line{};
    auto last = this->getInnerBegin();

    line.push_back(QString(R"([%1])").arg(this->getInnerBegin().toString(true, false, true)));
    for (const auto &syl: this->_syl_s) {
        if (syl->getBegin() > last) {
            line.push_back(QString("<%1>\u200B").arg(last.toString(true, false, true)));
        }
        line.push_back(syl->toSPL());
        if (not syl->isText()) last = syl->getEnd();
    }
    line.push_back(QString(R"([%1])").arg(this->getInnerEnd().toString(true, false, true)));

    QStringList text {};

    auto line_text = line.join("");

    if (this->_is_bg) {
        line_text.insert(line_text.indexOf('>') + 1, '(');
        line_text.insert(line_text.lastIndexOf('['), ')');
    }
    text.push_back(line_text);

    for (const auto &ptr: this->_transliteration | std::views::values) {
        if (const auto roma_pair = std::get_if<std::pair<QString, std::shared_ptr<QString>>>(ptr.get())) {
            if (not roma_pair->first.isEmpty()) text.push_back(roma_pair->first);
        } elif (const auto roma_line = std::get_if<LyricLine>(ptr.get())) {
            text.push_back(roma_line->toTXT());
        } elif (const auto roma_str = std::get_if<QString>(ptr.get())) {
            text.push_back(*roma_str);
        }
    }

    for (const auto &ptr: this->_translation | std::views::values) {
        if (const auto trans_pair = std::get_if<std::pair<QString, std::shared_ptr<QString>>>(ptr.get())) {
            if (not trans_pair->first.isEmpty()) text.push_back(trans_pair->first);
        } elif (const auto trans_line = std::get_if<LyricLine>(ptr.get())) {
            text.push_back(trans_line->toTXT());
        } elif (const auto trans_str = std::get_if<QString>(ptr.get())) {
            text.push_back(*trans_str);
        }
    }

    if (this->_bg_line) text.append(this->_bg_line->toSPL());

    return text.join("\n");
}
