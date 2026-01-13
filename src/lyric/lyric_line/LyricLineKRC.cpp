//
// Created by LEGION on 2025/12/24.
//

#include "lyricline.hpp"

QString LyricLine::toKRC() {
    QStringList text{};
    auto last = this->getInnerBegin();

    text.push_back(QString(R"([%1,%2])")
        .arg(static_cast<int64_t>(this->getInnerBegin()))
        .arg(static_cast<int64_t>(this->getLineDuration())));
    for (const auto &syl: this->_syl_s) {
        if (syl->isText()) {
            text.push_back(QString("<%1,0,0>%2")
                .arg(static_cast<int64_t>(last))
                .arg(syl->getText()));
        } else {
            text.push_back(syl->toKRC(this->getInnerBegin()));
            last = syl->getEnd();
        }
    }

    return text.join("");
}

QString LyricLine::getSubKRC(SubType role, const QString &lang) {
    QStringList text{};
    if (role == SubType::Translation) {
        if (this->_translation.contains(lang)) {
            const auto &ptr = this->_translation[lang];
            if (const auto sub_line = std::get_if<LyricLine>(ptr.get())) {
                text.push_back(QString(R"("%1")").arg(sub_line->toTXT()));
            } elif (const auto sub_pair = std::get_if<std::pair<QString, std::shared_ptr<QString>>>(ptr.get())) {
                text.push_back(QString(R"("%1")").arg(sub_pair->first));
            }
        } else {
            text.push_back(QString(R"("")"));
        }
    } else {
        if (this->_translation.contains(lang)) {
            const auto &ptr = this->_translation[lang];
            if (const auto sub_line = std::get_if<LyricLine>(ptr.get())) {
                int i = 0;
                for (const auto &syl: sub_line->_syl_s) {
                    while (i < this->_syl_s.size() and *this->_syl_s[i] != *syl) {
                        text.push_back(QString(R"("")"));
                        ++i;
                    }
                    text.push_back(QString(R"("%1")").arg(syl->getText()));
                    ++i;
                }
            } elif (const auto sub_pair = std::get_if<std::pair<QString, std::shared_ptr<QString>>>(ptr.get())) {
                text.push_back(QString(R"("%1")").arg(sub_pair->first));
            }
        } else {
            text.push_back(QString(R"("")"));
        }
    }
    return text.join(",");
}
