//
// Created by LEGION on 2025/12/19.
//

#include "LyricLine.h"

QString LyricLine::toASS(const QString& role) const {
    QStringList line{};

    line.append(R"(Dialogue: 0)");
    line.append(this->getInnerBegin().toString(true, true, true));
    line.append(this->getInnerEnd().toString(true, true, true));
    line.append(role);
    line.append(QString(R"(%1 %2)")
        .arg(this->_is_bg ? R"(x-bg)" : "")
        .arg(this->_is_duet ? R"(x-duet)" : "")
        .trimmed());
    line.append(R"(0,0,0,karaoke)");

    QStringList text{};
    LyricTime last = this->getInnerBegin();

    for (const auto &syl: this->_syl_s) {
        if (syl->getBegin() > last)
            text.append(QString(R"({\k%1})").arg(static_cast<int64_t>(syl->getDuration()) / 10));
        line.append(syl->toASS());
        last = syl->getEnd();
    }
    line.append(text.join(""));

    QStringList ass{};
    ass.append(line.join(","));
    if (not this->_transliteration.empty())
        for (const auto& [lang, sub_line]: this->_translation) {
            const auto ptr = sub_line.get();
            if (const auto trans_text = std::get_if<QString>(ptr)) {
                ass.append(QString(R"(Dialogue: 0,%1,%2,roma,%3,0,0,0,,%4)")
                    .arg(this->getInnerBegin().toString(true, true, true))
                    .arg(this->getInnerEnd().toString(true, true, true))
                    .arg(QString("x-lang:") + lang)
                    .arg(*trans_text));
            } elif (const auto trans_pair = std::get_if<std::pair<QString, std::shared_ptr<QString>>>(ptr)) {
                if (not trans_pair->first.isEmpty()) {
                    ass.append(QString(R"(Dialogue: 0,%1,%2,roma,%3,0,0,0,,%4)")
                        .arg(this->getInnerBegin().toString(true, true, true))
                        .arg(this->getInnerEnd().toString(true, true, true))
                        .arg(QString("x-lang:") + lang)
                        .arg(trans_pair->first));
                }
            } elif (const auto trans_line = std::get_if<LyricLine>(ptr)) {
                ass.append(trans_line->toASS("roma"));
            }
        }
    if (not this->_translation.empty()) {
        for (const auto& [lang, sub_line]: this->_translation) {
            const auto ptr = sub_line.get();
            if (const auto trans_text = std::get_if<QString>(ptr)) {
                ass.append(QString(R"(Dialogue: 0,%1,%2,ts,%3,0,0,0,,%4)")
                    .arg(this->getInnerBegin().toString(true, true, true))
                    .arg(this->getInnerEnd().toString(true, true, true))
                    .arg(QString("x-lang:") + lang)
                    .arg(*trans_text));
            } elif (const auto trans_pair = std::get_if<std::pair<QString, std::shared_ptr<QString>>>(ptr)) {
                if (not trans_pair->first.isEmpty()) {
                    ass.append(QString(R"(Dialogue: 0,%1,%2,ts,%3,0,0,0,,%4)")
                        .arg(this->getInnerBegin().toString(true, true, true))
                        .arg(this->getInnerEnd().toString(true, true, true))
                        .arg(QString("x-lang:") + lang)
                        .arg(trans_pair->first));
                }
            } elif (const auto trans_line = std::get_if<LyricLine>(ptr)) {
                ass.append(trans_line->toASS("ts"));
            }
        }
    }
    if (this->_bg_line)
        ass.append(this->_bg_line->toASS());

    return ass.join("\n");
}
