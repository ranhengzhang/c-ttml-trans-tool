//
// Created by LEGION on 2025/12/23.
//

#include "LyricSyl.h"

QString LyricSyl::toQRC() const {
    return QString(R"(%1(%2,%3))")
    .arg(this->_text)
    .arg(static_cast<int64_t>(this->_begin))
    .arg(std::max(0LL, static_cast<int64_t>(this->getDuration())));
}
