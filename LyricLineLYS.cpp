//
// Created by LEGION on 2025/12/24.
//

#include "LyricLine.h"

QString LyricLine::toLYS(const QString &lang, bool have_bg, bool have_duet) {
    int property = ((int)have_bg + (int)this->_is_bg) * 3 + static_cast<int>(have_duet) + static_cast<int>(this->_is_duet);
    QStringList text{};

    text.push_back(QString(R"([%1])").arg(property));
    if (this->_bg_line) {
        text.push_back("\n");
        text.push_back(this->_bg_line->toLYS(lang, have_bg, have_duet));
    }

    return text.join("");
}
