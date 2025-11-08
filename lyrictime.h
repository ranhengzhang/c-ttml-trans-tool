//
// Created by LEGION on 25-4-27.
//

#ifndef LYRICTIME_H
#define LYRICTIME_H

#include <qstring.h>

class LyricTime {
public:
    [[nodiscard]] static LyricTime parse(const QString &str, bool *ok);

    bool operator==(const LyricTime &rhs) const;

    bool operator!=(const LyricTime &rhs) const;

    int64_t operator-(const LyricTime &rhs) const;

    LyricTime operator+(int64_t rhs) const;

    friend bool operator<(const LyricTime &lhs, const LyricTime &rhs);

    friend bool operator<=(const LyricTime &lhs, const LyricTime &rhs);

    friend bool operator>(const LyricTime &lhs, const LyricTime &rhs);

    friend bool operator>=(const LyricTime &lhs, const LyricTime &rhs);

    void offset(int64_t count);

    [[nodiscard]] QString toString(bool to_long, bool to_short, bool to_dot) const;

    [[nodiscard]] int64_t getCount() const;

    [[nodiscard]] LyricTime toShort() const;

private:
    int64_t _count{0};
};


#endif //LYRICTIME_H
