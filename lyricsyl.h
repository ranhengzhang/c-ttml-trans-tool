//
// Created by LEGION on 25-4-27.
//

#ifndef LYRICSYL_H
#define LYRICSYL_H

#include <QDomElement>

#include "lyrictime.h"


class LyricSyl {
public:
    enum class NormalizeOption {
        FRONT,
        BACK
    };

    [[nodiscard]] static LyricSyl parse(const QDomElement &span, bool *ok);

    [[nodiscard]] static LyricSyl parse(const QString &text, LyricTime begin, LyricTime end, bool *ok);

    void setBegin(LyricTime begin);

    void setEnd(LyricTime end);

    [[nodiscard]] LyricTime getBegin() const;

    [[nodiscard]] LyricTime getEnd() const;

    [[nodiscard]] QString getText() const;

    void normalize(NormalizeOption opt);

    [[nodiscard]] QString toTTML() const;

    [[nodiscard]] QString toASS() const;

    [[nodiscard]] QString toSPL() const;

    [[nodiscard]] QString toQRC() const;

    [[nodiscard]] QString toYRC() const;

    [[nodiscard]] QString toKRC(const LyricTime &begin) const;

private:
    static QRegularExpression _before;
    static QRegularExpression _after;

    QString _text{};
    LyricTime _begin{};
    LyricTime _end{};
};


#endif //LYRICSYL_H
