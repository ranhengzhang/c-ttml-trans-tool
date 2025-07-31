//
// Created by LEGION on 25-4-27.
//

#ifndef LYRIC_H
#define LYRIC_H

#include "lyricline.h"
#include "lyrictime.h"

class Lyric {
public:
    [[nodiscard]] static Lyric parse(const QDomElement &tt, bool *ok);

    [[nodiscard]] static Lyric fromLYS(QStringList orig, QStringList ts, bool *ok);

    [[nodiscard]] static Lyric fromYRC(QStringList orig, QStringList ts, bool *ok);

    [[nodiscard]] static Lyric fromQRC(QStringList orig, QStringList ts, QStringList roman, bool *ok);

    [[nodiscard]] QString getTitle(const QString &postfix = "");

    [[nodiscard]] QMap<QString, QStringList> getPresetMeta();

    [[nodiscard]] QList<QPair<QString, QString>> getExtraMeta();

    [[nodiscard]] bool haveDuet() const;

    [[nodiscard]] bool haveBg() const;

    [[nodiscard]] bool haveRoman() const;

    [[nodiscard]] QList<QString> getLangs() const;

    [[nodiscard]] QString toTTML();

    [[nodiscard]] QString toASS();

    [[nodiscard]] QString toLRC(const QString &extra);

    [[nodiscard]] QString toSPL();

    [[nodiscard]] QPair<QString, QString> toLYS(const QString &lang);

    [[nodiscard]] QMap<QString, QString> toQRC(const QString &lang);

    [[nodiscard]] QPair<QString, QString> toYRC(const QString &lang);

    [[nodiscard]] QString toKRC(const QString &lang);

    [[nodiscard]] QString toTXT();

    static QList<QPair<QString, QString>> presetMetas;

private:
    struct LyricPart {
        uint8_t _count{};
        QString _songPart{};
    };

    static QRegularExpression _header;
    static QRegularExpression _lys_role;
    static QRegularExpression _line_time;
    static QRegularExpression _syl_time;
    static QStringList presetKey;
    QList<QPair<QString, QString>> _meta_s{};
    QList<LyricLine> _line_s{};
    QList<LyricPart> _part_s{};
    bool _have_duet{false};
    bool _have_bg{false};
    bool _have_roman{false};
    QSet<QString> _lang_s{};
    LyricTime _begin{};
    LyricTime _end{};

    [[nodiscard]] QString getTTMLHead();

    [[nodiscard]] QString getTTMLBody();

    [[nodiscard]] QString getASSHead();

    [[nodiscard]] QString getASSBody();

    [[nodiscard]] QString getLRCHead();

    [[nodiscard]] QString getLRCBody(const QString &extra);

    [[nodiscard]] QString getSPLBody();

    [[nodiscard]] QPair<QString, QString> getLYSBody(const QString &lang);

    [[nodiscard]] QMap<QString, QString> getQRCBody(const QString &lang);

    [[nodiscard]] QStringList getKRCLang(const QString &lang);

    [[nodiscard]] QString getKRCBody();

    [[nodiscard]] LyricTime getDur();
};


#endif //LYRIC_H
