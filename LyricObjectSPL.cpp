//
// Created by LEGION on 2025/12/21.
//

#include "LyricObject.h"

QString LyricObject::toSPL() {
    QStringList text{};

    for (auto &line: this->_line_s) text.push_back(line.toSPL());

    return this->getLRCHead() + "\n\n" + text.join("\n");
}
