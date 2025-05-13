//
// Created by LEGION on 25-4-27.
//

#include "lyrictime.h"

#include <qregularexpression.h>

QRegularExpression LyricTime::_timeReg =
        QRegularExpression(R"(((\d+):)?(\d{2,}):(\d{2})[:\.](\d{2,3}))");

LyricTime LyricTime::parse(const QString &str, bool *ok) {
    if (str.isEmpty()) {
        *ok = false;
        return {};
    }

    LyricTime time{};

    const auto match = LyricTime::_timeReg.match(str);

    if (!match.hasMatch()) {
        *ok = false;
        return {};
    }

    const auto hours = match.captured(2).isEmpty() ? 0 : match.captured(2).toInt();
    const auto minutes = match.captured(3).toInt();
    const auto seconds = match.captured(4).toInt();
    auto milliseconds = match.captured(5).toInt();

    if (2 == match.captured(5).length()) milliseconds *= 10;
    // 计算总毫秒数
    time._count = hours * 3600 * 1000 // 小时转毫秒
                  + minutes * 60 * 1000 // 分钟转毫秒
                  + seconds * 1000 // 秒转毫秒
                  + milliseconds; // 毫秒部分

    *ok = true;
    return time;
}

void LyricTime::offset(const int64_t count) {
    this->_count += count;
}

QString LyricTime::toString(bool to_long, const bool to_short, const bool to_dot) const {
    auto count = this->_count;
    QStringList time{};

    if (count >= 60 * 60 * 1000) to_long = true;
    if (to_short) time.push_front(QString::asprintf("%02lld", count % 1000 / 10));
    else time.push_front(QString::asprintf("%03lld", count % 1000));
    count /= 1000;

    time.push_front(to_dot ? "." : ":");

    time.push_front(QString::asprintf("%02lld", count % 60));
    count /= 60;

    time.push_front(":");

    if (to_long) {
        time.push_front(QString::asprintf("%02lld", count % 60));
        time.push_front(":");
        time.push_front(QString::asprintf("%lld", count / 60));
    } else {
        time.push_front(QString::asprintf("%02lld", count));
    }

    return time.join("");
}

int64_t LyricTime::getCount() const {
    return this->_count;
}

LyricTime LyricTime::toShort() const {
    LyricTime time{};
    time._count = this->_count - this->_count % 10;
    return time;
}

bool LyricTime::operator==(const LyricTime &rhs) const {
    return _count == rhs._count;
}

bool LyricTime::operator!=(const LyricTime &rhs) const {
    return !(rhs == *this);
}

int64_t LyricTime::operator-(const LyricTime &rhs) const {
    return this->_count - rhs._count;
}

LyricTime LyricTime::operator+(const int64_t rhs) const {
    LyricTime time{};
    time._count = this->_count + rhs;
    return time;
}

bool operator<(const LyricTime &lhs, const LyricTime &rhs) {
    return lhs._count < rhs._count;
}

bool operator<=(const LyricTime &lhs, const LyricTime &rhs) {
    // ReSharper disable once CppRedundantComplexityInComparison
    return !(rhs < lhs);
}

bool operator>(const LyricTime &lhs, const LyricTime &rhs) {
    return rhs < lhs;
}

bool operator>=(const LyricTime &lhs, const LyricTime &rhs) {
    return !(lhs < rhs);
}
