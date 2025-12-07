//
// Created by LEGION on 2025/11/21.
//

#include "lyrictimeplus.h"

#include <QStringList>

std::pair<LyricTimePlus, bool> LyricTimePlus::parse(const QString &str) {
    if (str.isEmpty()) {
        return {{}, false};
    }

    LyricTimePlus time{};
    QStringList matches{};
    QString match{};

    for (int i = str.length() - 1; i >= 0; --i) {
        if (str[i] == '.' || str[i] == ':') {
            matches.push_back(match);
            matches.push_back(str[i]);
            match.clear();
        } else if (str[i].isDigit()) {
            match.push_front(str[i]);
        } else {
            return {{}, false};
        }
    }
    matches.push_back(match);

    const auto hours = matches.length() > 5 ? matches.at(6).toInt() : 0;
    const auto minutes = matches.length() > 3 ? matches.at(4).toInt() : 0;
    const auto seconds = matches.at(2).toInt();
    auto milliseconds = matches.at(0).toInt();

    if (2 == matches.at(0).length()) milliseconds *= 10; // 处理厘秒格式
    // 计算总毫秒数
    time._count = hours * 3600 * 1000 // 小时转毫秒
                  + minutes * 60 * 1000 // 分钟转毫秒
                  + seconds * 1000 // 秒转毫秒
                  + milliseconds; // 毫秒部分

    return {time, true};
}

void LyricTimePlus::offset(const int64_t count) {
    this->_count += count;
}

QString LyricTimePlus::toString(bool to_long, bool to_centi, bool to_dot) const {
    auto count = this->_count;
    QStringList time{};

    if (count >= 60 * 60 * 1000) to_long = true;
    if (to_centi) time.push_front(QString::asprintf("%02lld", count % 1000 / 10));
    else time.push_front(QString::asprintf("%03lld", count % 1000));
    count /= 1000;

    time.push_front(to_dot ? "." : ":");

    time.push_front(QString::asprintf(to_long || count >= 60 ? "%02lld" : "%lld", count % 60));
    count /= 60;

    if (count || to_long) {
        time.push_front(":");

        time.push_front(QString::asprintf(to_long || count >= 60 ? "%02lld" : "%lld", count % 60));

        if (count >= 60 || to_long) {
            time.push_front(":");
            time.push_front(QString::asprintf("%lld", count / 60));
        }
    }

    return time.join("");
}

LyricTimePlus::operator int64_t() const {
    return this->_count;
}

LyricTimePlus LyricTimePlus::toShort() const {
    LyricTimePlus time{};
    time._count = this->_count - this->_count % 10;
    return time;
}
