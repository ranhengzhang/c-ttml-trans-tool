//
// Created by LEGION on 2025/12/23.
//

#include "LyricSyl.h"

QString LyricSyl::toSPL() const {
    return this->isText() ? this->_text : QString(R"(<%1>%2)").arg(this->_begin.toString(true, false, true)).arg(this->_text);
}
