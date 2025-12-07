//
// Created by LEGION on 2025/11/18.
//

#ifndef TTML_TOOL_LYRICPLUS_H
#define TTML_TOOL_LYRICPLUS_H

#include <QString>
#include <QStringList>

#include "lyriclineplus.h"

class LyricPlus {
public:
    [[nodiscard]] static std::pair<LyricPlus, bool> parse(const QDomElement &tt);

    [[nodiscard]] QString toTTML();

private:
    struct LyricMeta {
        QString key{};
        QString value{};
    };

    struct LyricPart {
        uint8_t count{};
        QString songPart{};
    };

    struct LyricTrans {
        QString target{};
        std::variant<std::pair<QString, QString>, LyricLinePlus> content{};
    };

    QString lang{};
    QStringList _song_writer_s{}; // 歌词作者
    QList<LyricMeta> _meta_data_s{}; // 元数据
    QList<LyricPart> _song_part_s{}; // 歌曲分段
    QList<LyricLinePlus> _line_s{}; // 歌词行
    QMap<QString, QList<LyricTrans>> _transliteration_s{}; // 音译
    QMap<QString, std::pair<bool, QList<LyricTrans>>> _translation_s{}; // 翻译

    bool _have_other{};
};


#endif //TTML_TOOL_LYRICPLUS_H
