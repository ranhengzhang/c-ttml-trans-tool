//
// Created by LEGION on 2025/12/14.
//

#ifndef TTML_TOOL_LYRIC_H
#define TTML_TOOL_LYRIC_H

#include "../lyric_line/lyricline.hpp"
#include "../utils/utils.hpp"

#include <variant>

#include <QString>
#include <QMap>
#include <QList>

class LyricObject {
public:
    using Status = utils::Status;

    using SubType = utils::SubType;

    using LyricTrans = utils::LyricTrans;

    [[nodiscard]] static std::pair<LyricObject, Status> fromTTML(const QString &ttml);

    [[nodiscard]] QString toTTML();

    [[nodiscard]] QString toASS();

    [[nodiscard]] QString getLRCHead();

    /**
     * 转换为 LRC 歌词，以 Walkman 播放效果为准
     * @param extra 附加行类型 + 语言 @code "[x-trans/x-roman]:lang"</code> @endcode
     * @return @code [ orig_file, trans_file ] @endcode
     */
    [[nodiscard]] QString toLRC(const QString &extra);

    [[nodiscard]] QString getSubLRC(std::map<QString, std::shared_ptr<LyricTrans>> &map);

    [[nodiscard]] QString toSPL();

    /**
     * 转换为 QRC 歌词
     * @param ts_lang 翻译语言
     * @param roma_lang 音译语言
     * @return @code [ orig_file, trans_file, roma_file ] @endcode
     */
    [[nodiscard]] std::tuple<QString, QString, QString> toQRC(const QString &ts_lang, const QString &roma_lang);

    [[nodiscard]] QString getSubQRC(std::map<QString, std::shared_ptr<LyricTrans>> &map);

    /**
     * 转换为 LYS 歌词
     * @param lang 翻译语言
     * @return @code [ orig_file, trans_file ] @endcode
     */
    [[nodiscard]] std::pair<QString, QString> toLYS(const QString &lang);

    /**
     * 转换为 YRC 歌词
     * @param lang 翻译语言
     * @return @code [ orig_file, trans_file ] @endcode
     */
    [[nodiscard]] std::pair<QString, QString> toYRC(const QString &lang);

    [[nodiscard]] QString toKRC(const QString &ts_lang, const QString &roma_lang);

    [[nodiscard]] QString getSubKRC(SubType role, const QString &lang);

    [[nodiscard]] QString toTXT();

    [[nodiscard]] QMap<QString, QStringList> getPresetMeta();

    [[nodiscard]] QList<QPair<QString, QString>> getExtraMeta();

    [[nodiscard]] QStringList getSongWriter();

    /**
     * @brief 接收 Line 上报的附加行容器
     * @param role 类型
     * @param trans_map 附加行
     * @param line_key 行号
     */
    void appendSubLine(SubType role, const std::map<QString, std::shared_ptr<LyricTrans>>& trans_map, const QString& line_key);

    [[nodiscard]] QString getTitle(const QString &postfix = "");

    [[nodiscard]] QList<QString> getSubLangs() const;

    [[nodiscard]] QList<QString> getTransLangs() const;

    [[nodiscard]] QList<QString> getRomaLangs() const;

    [[nodiscard]] QString getLang() const;

    [[nodiscard]] LyricTime getDur();

    [[nodiscard]] bool haveDuet() const;

    [[nodiscard]] bool haveBg() const;

    [[nodiscard]] bool haveRoman() const;

    [[nodiscard]] bool haveTrans() const;

private:
    QString getAssHeader();

    struct LyricMeta {
        QString key{};
        QString value{};
        bool operator==(const LyricMeta&) const = default;
    };

    struct LyricPart {
        uint8_t count{};
        QString song_part{};
    };

    QString _lang{};

    QStringList _song_writer_s{}; // 歌词作者

    QList<LyricMeta> _meta_data_s{}; // 元数据

    QList<LyricPart> _song_part_s{}; // 歌曲分段

    QList<LyricLine> _line_s{}; // 歌词行

    /**
     * @brief 翻译\n
     * @code { lang:string, trans: [ is_word:boolean, list: { key:string, line:ref<LyricTrans> } ] } @endcode
     */
    QMap<QString, std::pair<bool, std::map<QString, std::shared_ptr<LyricTrans>>>> _translation_s{};

    /**
     * @brief 音译\n
     * @code { lang:string, list: { key:string, line:ref<LyricTrans> } } @endcode
     */
    QMap<QString, std::map<QString, std::shared_ptr<LyricTrans>>> _transliteration_s{};

    bool _have_duet{};

    bool _have_bg{};
};

#endif //TTML_TOOL_LYRIC_H
