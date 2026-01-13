//
// Created by LEGION on 2025/12/14.
//


#include "LyricTime.hpp"

#include <QStringList>

#include "../utils/utils.hpp"

LyricTime::LyricTime(const int64_t count): _count(count) {
}

std::pair<LyricTime, bool> LyricTime::parse(const QString &str) {
    if (str.isEmpty()) {
        return {{}, false};
    }

    LyricTime time{};
    QStringList matches{};
    QString match{};

    for (int i = str.length() - 1; i >= 0; --i) {
        if (str[i] == '.' || str[i] == ':') {
            matches.push_back(match);
            matches.push_back(str[i]);
            match.clear();
        } elif (str[i].isDigit()) {
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

void LyricTime::offset(const int64_t count) {
    this->_count += count;
}

QString LyricTime::toString(const bool to_long, const bool to_centi, const bool to_dot) const {
    auto count = this->_count;
    QStringList time{};

    if (to_centi) time.push_front(QString::asprintf("%02lld", count % 1000 / 10));
    else time.push_front(QString::asprintf("%03lld", count % 1000));
    count /= 1000;

    time.push_front(to_dot ? "." : ":");

    time.push_front(QString::asprintf(to_long or count >= 60 ? "%02lld" : "%lld", count % 60));
    count /= 60;

    if (count or to_long) {
        time.push_front(":");

        time.push_front(QString::asprintf(to_long or count >= 60 ? "%02lld" : "%lld", count % 60));

        if (count >= 60) {
            time.push_front(":");
            time.push_front(QString::asprintf("%lld", count / 60));
        }
    }

    return time.join("");
}

LyricTime::operator int64_t() const {
    return this->_count;
}

LyricTime LyricTime::toShort() const {
    LyricTime time{};
    time._count = this->_count - this->_count % 10;
    return time;
}
