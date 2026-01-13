//
// Created by LEGION on 2025/12/24.
//

#include "lyricline.hpp"

QString LyricLine::toYRC() {
    QStringList text{};

    text.push_back(QString(R"([%1,%2]%3)")
        .arg(static_cast<int64_t>(this->getInnerBegin()))
        .arg( static_cast<int64_t>(this->getInnerDuration()))
        .arg(this->toInnerYRC()));
    if (this->_bg_line) {
        text.push_back("\n");
        text.push_back(this->_bg_line->toYRC());
    }

    return text.join("");
}

QString LyricLine::toInnerYRC() {
    QStringList text{};
    auto last = this->getInnerBegin();

    for (int i = 0; i < this->_syl_s.size(); i++) {
        const auto &syl = this->_syl_s[i];
        if (syl->isText()) {
            text.push_back(QString(R"(<%1,0,0>%2)")
                .arg(static_cast<int64_t>(last))
                .arg(syl->getText()));
        } else {
            text.push_back(syl->toYRC());
            last = syl->getEnd();
        }
    }

    return text.join("");
}
