//
// Created by LEGION on 2025/12/21.
//

#include "LyricObject.h"

QString LyricObject::getLRCHead() {
    QMap<QString, QStringList> meta;

    for (const auto &[key, value]: _meta_data_s) {
        if (key == R"(musicName)") {
            meta[R"(ti)"].append(value);
        } else if (key == R"(artists)") {
            meta[R"(ar)"].append(value);
        } else if (key == R"(album)") {
            meta[R"(al)"].append(value);
        }
    }

    QStringList text{};

    for (const auto &value: meta[R"(ti)"])
        text.append(QString(R"([ti:%1])").arg(value));
    for (const auto &value: meta[R"(ar)"])
        text.append(QString(R"([ar:%1])").arg(value));
    for (const auto &value: meta[R"(al)"])
        text.append(QString(R"([al:%1])").arg(value));

    return text.join("\n");
}

QString LyricObject::toLRC(const QString &extra) {
    QStringList text{};
    QStringList ext{};
    auto last_end = this->_line_s.front().getLineBegin();

    text.push_back("[00:00.00]");
    for (auto&line: this->_line_s) {
        if (not ext.isEmpty()) {
            auto time = line.getLineBegin() - last_end > 500 ? last_end : line.getLineBegin();
            for (auto &ext_line: ext) {
                text.push_back(QString(R"([%1]%2)")
                    .arg(time.toString(false, true, true))
                    .arg(ext_line));
            }
            ext.clear();
        }
        if (line.getLineBegin() - last_end > 500) {
            text.push_back(QString(R"([%1])")
                .arg(last_end.toString(false, true, true)));
        }
        auto [orig_line, sub_line] = line.toLRC(extra);
        text.append(orig_line);
        if (not sub_line.isEmpty()) ext.append(sub_line);
        last_end = line.getLineEnd();
    }
    text.push_back(QString(R"([%1])").arg(this->_line_s.back().getLineEnd().toString(false, true, true)));

    return text.join("\n");
}

QString LyricObject::getSubLRC(std::map<QString, std::shared_ptr<LyricTrans>> &map) {
    QStringList text{};

    for (auto &[key, roma_ptr]: map) {
        if (const auto pair_ptr = std::get_if<std::pair<QString, std::shared_ptr<QString>>>(roma_ptr.get())) {
            auto orig_line = *std::ranges::find_if(this->_line_s, [&key](auto &line) { return line.getKey() == key; });
            text.push_back(QString(R"([%1]%2)")
                .arg(orig_line.getInnerBegin().toString(false, false, true))
                .arg(pair_ptr->first));
            if (orig_line.haveBgLine() and pair_ptr->second) {
                text.push_back(QString(R"([%1]%2)")
                    .arg(orig_line.getBgLine()->getInnerBegin().toString(false, false, true))
                    .arg(*pair_ptr->second));
            }
        } elif (const auto line_ptr = std::get_if<LyricLine>(roma_ptr.get())) {
            text.push_back(line_ptr->toLRC());
            if (line_ptr->haveBgLine()) {
                text.push_back(line_ptr->getBgLine()->toLRC());
            }
        }
    }

    return text.join("\n");
}
