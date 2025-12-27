//
// Created by LEGION on 2025/12/19.
//

#include "LyricLine.h"

QString LyricLine::toASS(const QString& role, LyricTime parent_begin, LyricTime parent_end) const {
    QStringList line{};

    const LyricTime line_begin = parent_begin == -1 ? this->getInnerBegin() : parent_begin;
    const LyricTime line_end = parent_end == -1 ? this->getInnerEnd() : parent_end;
    line.push_back(R"(Dialogue: 0)");
    line.push_back(line_begin.toString(true, true, true));
    line.push_back(line_end.toString(true, true, true));
    line.push_back(role);
    line.push_back(QString(R"(%1 %2)")
        .arg(this->_is_bg ? R"(x-bg)" : "")
        .arg(this->_is_duet ? R"(x-duet)" : "")
        .trimmed());
    line.push_back(R"(0,0,0,karaoke)");

    QStringList text{};
    LyricTime last = this->getInnerBegin();

    for (const auto &syl: this->_syl_s) {
        if (syl->getBegin() > last)
            text.push_back(QString(R"({\k%1})").arg(static_cast<int64_t>(syl->getBegin() / 10 - last / 10)));
        text.push_back(syl->toASS());
        last = syl->getEnd();
    }
    line.push_back(text.join(""));

    QStringList ass{};
    ass.push_back(line.join(","));
    if (not this->_transliteration.empty())
        for (const auto& [lang, sub_line]: this->_transliteration) {
            const auto ptr = sub_line.get();
            if (const auto trans_text = std::get_if<QString>(ptr)) {
                ass.push_back(QString(R"(Dialogue: 0,%1,%2,roma,%3,0,0,0,karaoke,%4)")
                    .arg(line_begin.toString(true, true, true))
                    .arg(line_end.toString(true, true, true))
                    .arg(QString("x-lang:") + lang)
                    .arg(*trans_text));
            } elif (const auto trans_pair = std::get_if<std::pair<QString, std::shared_ptr<QString>>>(ptr)) {
                if (not trans_pair->first.isEmpty()) {
                    ass.push_back(QString(R"(Dialogue: 0,%1,%2,roma,%3,0,0,0,karaoke,%4)")
                        .arg(line_begin.toString(true, true, true))
                        .arg(line_end.toString(true, true, true))
                        .arg(QString("x-lang:") + lang)
                        .arg(trans_pair->first));
                }
            } elif (const auto trans_line = std::get_if<LyricLine>(ptr)) {
                ass.push_back(trans_line->toASS("roma", line_begin, line_end));
            }
        }
    if (not this->_translation.empty()) {
        for (const auto& [lang, sub_line]: this->_translation) {
            const auto ptr = sub_line.get();
            if (const auto trans_text = std::get_if<QString>(ptr)) {
                ass.push_back(QString(R"(Dialogue: 0,%1,%2,ts,%3,0,0,0,karaoke,%4)")
                    .arg(line_begin.toString(true, true, true))
                    .arg(line_end.toString(true, true, true))
                    .arg(QString("x-lang:") + lang)
                    .arg(*trans_text));
            } elif (const auto trans_pair = std::get_if<std::pair<QString, std::shared_ptr<QString>>>(ptr)) {
                if (not trans_pair->first.isEmpty()) {
                    ass.push_back(QString(R"(Dialogue: 0,%1,%2,ts,%3,0,0,0,karaoke,%4)")
                        .arg(line_begin.toString(true, true, true))
                        .arg(line_end.toString(true, true, true))
                        .arg(QString("x-lang:") + lang)
                        .arg(trans_pair->first));
                }
            } elif (const auto trans_line = std::get_if<LyricLine>(ptr)) {
                ass.push_back(trans_line->toASS("ts", line_begin, line_end));
            }
        }
    }
    if (this->_bg_line and role == "orig")
        ass.push_back(this->_bg_line->toASS());

    return ass.join("\n");
}
