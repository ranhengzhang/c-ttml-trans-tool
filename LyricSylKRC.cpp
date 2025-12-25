//
// Created by LEGION on 2025/12/24.
//

#include "LyricSyl.h"

QString LyricSyl::toKRC(const LyricTime &line_begin) const {
    return QString(R"(<%1,%2,0>%3)")
    .arg(static_cast<int64_t>(this->_begin - line_begin))
    .arg(std::max(0LL, static_cast<int64_t>(this->getDuration())))
    .arg(this->_text);
}
