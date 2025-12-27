//
// Created by LEGION on 2025/12/23.
//

#include "LyricLine.h"

std::pair<QString, QStringList> LyricLine::toLRC(const QString &extra) {
    QString orig_line{};
    QStringList sub_line{};

    orig_line = QString(R"([%1]%2)")
    .arg(this->getLineBegin().toString(true, true, true))
    .arg(this->toTXT());
    if (this->_bg_line) sub_line.push_back(QString("(%1)").arg(this->_bg_line->toTXT()));

    std::shared_ptr<LyricTrans> sub_ptr{};
    if (extra.startsWith("x-roman") and this->_transliteration.contains( extra.split(":").last())) {
        sub_ptr = this->_transliteration[extra.split(":").last()];
    } elif (extra.startsWith("x-trans") and this->_translation.contains( extra.split(":").last())) {
        sub_ptr = this->_translation[extra.split(":").last()];
    }
    if (sub_ptr) {
        if (const auto trans_line = std::get_if<LyricLine>(sub_ptr.get())) {
            sub_line.push_back(trans_line->toTXT());
            if (trans_line->_bg_line) sub_line.push_back(QString("(%1)").arg(trans_line->_bg_line->toTXT()));
        } elif (const auto trans_pair = std::get_if<std::pair<QString, std::shared_ptr<QString>>>(sub_ptr.get())) {
            if (not trans_pair->first.isEmpty()) sub_line.push_back(trans_pair->first);
            if (trans_pair->second) sub_line.push_back(QString("(%1)").arg(*trans_pair->second));
        }
    }

    return {orig_line, sub_line};
}

QString LyricLine::toLRC() const {
    return QString(R"([%1]%2)")
            .arg(this->getInnerBegin().toString(true, false, true))
            .arg(this->toTXT());
}
