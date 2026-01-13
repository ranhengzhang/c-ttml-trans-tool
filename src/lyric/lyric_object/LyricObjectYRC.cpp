//
// Created by LEGION on 2025/12/21.
//

#include "LyricObject.hpp"

std::pair<QString, QString> LyricObject::toYRC(const QString &lang) {
    QStringList orig_text{};
    QString ts_text{};

    for (auto &line: this->_line_s) orig_text.push_back(line.toYRC());
    if (this->_translation_s.contains(lang)) {
        auto &[is_word, ts_map] = this->_translation_s[lang];
        ts_text = this->getSubLRC(ts_map);
    }

    return {this->getLRCHead() + "\n\n" + orig_text.join("\n"), ts_text};
}
