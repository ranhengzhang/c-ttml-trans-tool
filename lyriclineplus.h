//
// Created by LEGION on 2025/11/21.
//

#ifndef TTMLX_LYRICLINEPLUS_H
#define TTMLX_LYRICLINEPLUS_H

#include <QDomElement>
#include <QMap>

#include "lyricsylplus.h"


class LyricLinePlus {
public:
    [[nodiscard]] static std::pair<LyricLinePlus, bool> parse(const QDomElement &p, const LyricLinePlus *parent);

    void match(const LyricLinePlus &orig);

    [[nodiscard]] QString toTTML() const;

    [[nodiscard]] QString toInnerTTML(bool xmlns = false) const;

    [[nodiscard]] LyricTimePlus getBegin() const;

    [[nodiscard]] LyricTimePlus getEnd() const;

private:
    std::shared_ptr<LyricLinePlus> _bg_line{};
    QList<std::shared_ptr<LyricSylPlus>> _syl_s{};
    LyricTimePlus _begin{};
    LyricTimePlus _end{};
    QString _key{};
    bool _is_duet{false};
    bool _is_bg{false};
};


#endif //TTMLX_LYRICLINEPLUS_H
