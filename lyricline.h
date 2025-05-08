//
// Created by LEGION on 25-4-27.
//

#ifndef LYRICLINE_H
#define LYRICLINE_H

#include <QMap>
#include <QSet>

#include "lyricsyl.h"
#include "lyrictime.h"

class LyricLine {
public:
    [[nodiscard]] static LyricLine parse(const QDomElement &p, bool is_bg, bool *ok);

    [[nodiscard]] bool haveBg() const;

    [[nodiscard]] bool haveRoman() const;

    [[nodiscard]] LyricTime getBegin() const;

    [[nodiscard]] LyricTime getEnd() const;

    [[nodiscard]] QSet<QString> getLang() const;

    [[nodiscard]] QString toTTML(int num = -1);

    [[nodiscard]] QString toASS();

    [[nodiscard]] QString toTXT();

    [[nodiscard]] QString toLRC(const QString &extra, LyricTime next);

    [[nodiscard]] QString toSPL();

    [[nodiscard]] QPair<QString, QString> toLYS(bool have_bg, bool have_duet, const QString &lang);

    [[nodiscard]] QMap<QString, QString> toQRC(const QString &lang);

    [[nodiscard]] QPair<QString, QString> toYRC(const QString &lang);

private:
    std::shared_ptr<LyricLine> _bg_line{};
    QMap<QString, QString> _trans{};
    QString _roman{};
    QList<LyricSyl> _syl_s{};
    LyricTime _begin{};
    LyricTime _end{};
    bool _is_duet{false};
    bool _is_bg{false};
};


#endif //LYRICLINE_H
