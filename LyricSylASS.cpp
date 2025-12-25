//
// Created by LEGION on 2025/12/19.
//

#include "LyricSyl.h"

QString LyricSyl::toASS() const {
    const auto dur = static_cast<int64_t>(this->_end - this->_begin);
    return QString(R"({\k%1%2}%3)")
            .arg(std::max(dur, 0LL) / 10)
            .arg(this->isText() ? R"(\-T)" : (dur == 5 ? R"(\-Z)" : ""))
            .arg(this->_text);
}
