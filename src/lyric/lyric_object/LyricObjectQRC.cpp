//
// Created by LEGION on 2025/12/21.
//

#include "LyricLine.hpp"
#include "LyricObject.hpp"

#include <vector>
#include <algorithm>
#include <ranges>

std::tuple<QString, QString, QString> LyricObject::toQRC(const QString &ts_lang, const QString &roma_lang) {
    QStringList orig_text{};
    QString ts_text{};
    QString roma_text{};

    for (auto &orig_line: this->_line_s) orig_text.push_back(orig_line.toQRC());
    if (this->_translation_s.contains({ts_lang, false})) {
        auto &ts_map = this->_translation_s[{ts_lang, false}];
        ts_text = this->getSubLRC(ts_map);
    }
    elif (this->_translation_s.contains({ts_lang, true})) {
        auto &ts_map = this->_translation_s[{ts_lang, true}];
        ts_text = this->getSubQRC(ts_map);
    }
    if (this->_transliteration_s.contains(roma_lang)) {
        roma_text = this->getSubLRC(this->_transliteration_s[roma_lang]);
    }

    return {this->getLRCHead() + "\n\n" + orig_text.join("\n"), ts_text, roma_text};
}

QString LyricObject::getSubQRC(std::map<QString, std::shared_ptr<LyricTrans>> &map) {
    QStringList text{};

    // Collect and sort keys by length then default order
    std::vector<QString> keys;
    keys.reserve(map.size());
    for (const auto &[key, _]: map) {
        keys.push_back(key);
    }
    std::ranges::sort(keys, lyric::utils::compareKeysByLengthThenDefault);

    // Iterate through sorted keys
    for (const auto &key: keys) {
        const auto &roma_ptr = map.at(key);
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
