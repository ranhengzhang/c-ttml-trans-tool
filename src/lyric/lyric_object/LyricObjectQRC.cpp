//
// Created by LEGION on 2025/12/21.
//

#include "LyricObject.hpp"

std::tuple<QString, QString, QString> LyricObject::toQRC(const QString &ts_lang, const QString &roma_lang) {
    QStringList orig_text{};
    QString ts_text{};
    QString roma_text{};

    for (auto &orig_line: this->_line_s) orig_text.push_back(orig_line.toQRC());
    if (this->_translation_s.contains(ts_lang)) {
        auto &[is_word, ts_map] = this->_translation_s[ts_lang];
        ts_text = this->getSubLRC(ts_map);
    }
    if (this->_transliteration_s.contains(roma_lang)) {
        roma_text = this->getSubLRC(this->_transliteration_s[roma_lang]);
    }

    return {this->getLRCHead() + "\n\n" + orig_text.join("\n"), ts_text, roma_text};
}

QString LyricObject::getSubQRC(std::map<QString, std::shared_ptr<LyricTrans>> &map) {
    QStringList text{};

    for (auto &[key, roma_ptr]: map) {
        if (const auto pair_ptr = std::get_if<std::pair<QString, std::shared_ptr<QString>>>(roma_ptr.get())) {
            auto orig_line = *std::ranges::find_if(this->_line_s, [&key](auto &line) { return line.getKey() == key; });
            text.push_back(QString(R"([%1,%2]%3)")
                .arg(static_cast<int64_t>(orig_line.getInnerBegin()))
                .arg(static_cast<int64_t>(orig_line.getInnerDuration()))
                .arg(pair_ptr->first));
            if (orig_line.haveBgLine() and pair_ptr->second) {
                text.push_back(QString(R"([%1,%2]%3)")
                    .arg(static_cast<int64_t>(orig_line.getBgLine()->getInnerBegin()))
                    .arg(static_cast<int64_t>(orig_line.getBgLine()->getInnerDuration()))
                    .arg(*pair_ptr->second));
            }
        } elif (const auto line_ptr = std::get_if<LyricLine>(roma_ptr.get())) {
            text.push_back(line_ptr->toQRC());
        }
    }

    return text.join("\n");
}
