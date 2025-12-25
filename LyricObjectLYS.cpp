//
// Created by LEGION on 2025/12/21.
//

#include "LyricObject.h"

std::pair<QString, QString> LyricObject::toLYS(const QString &lang) {
    QStringList orig_text{};
    QString ts_text{};

    for (auto &line: this->_line_s) orig_text.push_back(line.toLYS(lang, this->haveBg(), this->haveDuet()));
    if (this->_translation_s.contains(lang)) {
        auto &[is_word, ts_map] = this->_translation_s[lang];
        ts_text = this->getSubLRC(ts_map);
    }

    return {this->getLRCHead() + "\n\n" + orig_text.join("\n"), ts_text};
}
