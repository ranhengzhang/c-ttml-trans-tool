//
// Created by LEGION on 2025/12/21.
//

#include "LyricLine.hpp"
#include "LyricObject.hpp"

QString LyricObject::toSPL() {
    QStringList lrc{};
    QList<std::tuple<bool, LyricTime, QString>> list{};

    for (auto &line: this->_line_s) list.append(line.toSPL());

    for (int i = 0; i < list.length(); ++i) {
        const auto &[isWord, time, text] = list[i];
        if (isWord) {
            int offset = 0;
            for (int j = i + 1; j < list.length(); ++j) {
                auto &[isWord2, time2, text2] = list[j];
                if (isWord2 and time == time2) {
                    time2 += ++offset;
                    list[j] = {isWord2, time2, text2};
                }
            }
        }
    }

    for (auto &[isWord, time, text]: list) {
        if (isWord) {
            lrc.append(QString(R"([%1]%2)")
                .arg(time.toString(true, false, true))
                .arg(text));
        } else {
            lrc.append(text);
        }
    }

    return this->getLRCHead() + "\n\n" + lrc.join("\n");
}
