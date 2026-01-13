//
// Created by LEGION on 2025/12/23.
//

#include "lyricline.hpp"

QString LyricLine::toQRC() {
    QStringList text{};

    text.push_back(QString(R"([%1,%2]%3)")
        .arg(static_cast<int64_t>(this->getInnerBegin()))
        .arg( static_cast<int64_t>(this->getInnerDuration()))
        .arg(this->toInnerQRC()));
    if (this->_bg_line) {
        text.push_back("\n");
        text.push_back(this->_bg_line->toQRC());
    }

    return text.join("");
}

QString LyricLine::toInnerQRC() {
    QStringList text{};
    auto last = this->getInnerBegin();

    for (int i = 0; i < this->_syl_s.size(); i++) {
        const auto &syl = this->_syl_s[i];
        if (syl->isText()) {
            text.push_back(QString(R"(%1(%2,0))")
                .arg(syl->getText())
                .arg(static_cast<int64_t>(last)));
        } else {
            text.push_back(syl->toQRC());
            last = syl->getEnd();
        }
    }
    if (this->_is_bg) {
        text[0] = QString("(") + text[0];
        text[text.size() - 1].insert(text[text.size() - 1].lastIndexOf("("), ")");
    }

    return text.join("");
}
