//
// Created by LEGION on 2025/12/14.
//

#ifndef LYRIC_PARSER_LYRICLINE_H
#define LYRIC_PARSER_LYRICLINE_H

#include <utility>
#include <map>

#include <QDomElement>
#include <QList>

#include "utils.hpp"
#include "LyricTime.hpp"

class LyricObject;
class LyricSyl;

class LyricLine {
public:
    using Status = lyric::utils::Status;

    using SubType = lyric::utils::SubType;

    using LyricTrans = lyric::utils::LyricTrans;

    friend LyricLine lyric::utils::normalizeBrackets(LyricLine &line);

    void setKey(const QString &key);

    void setIsDuet(bool is_duet);

    [[nodiscard]] static std::pair<LyricLine, Status> fromTTML(const QDomElement &p, LyricLine *parent, LyricObject &obj);

    [[nodiscard]] QString toTTML() const;

    [[nodiscard]] QString toInnerTTML(bool xmlns = false) const;

    [[nodiscard]] QString toTXT() const;

    [[nodiscard]] QString toASS(const QString& role = "orig", LyricTime parent_begin = {-1}, LyricTime parent_end = {-1}) const;

    [[nodiscard]] std::pair<QString, QStringList> toLRC(const QString &extra);

    [[nodiscard]] QString toLRC() const;

    [[nodiscard]] QString toSingleSPL();

    [[nodiscard]] QString toSPL();

    [[nodiscard]] QString toQRC();

    [[nodiscard]] QString toInnerQRC();

    [[nodiscard]] QString toLYS(const QString &lang, bool have_bg, bool have_duet);

    [[nodiscard]] QString toYRC();

    [[nodiscard]] QString toInnerYRC();

    [[nodiscard]] QString toKRC();

    [[nodiscard]] QString getSubKRC(SubType role, const QString &lang);

    /**
     * @brief 接收 Object 下发的附加行
     * @param role 类型
     * @param lang 语言
     * @param content 行内容
     */
    void appendSubLine(SubType role, const QString &lang, const std::shared_ptr<LyricTrans> &content);

    /**
     * @brief 将行内音节与主行的音节进行匹配
     * @param orig 主行
     */
    void match(const LyricLine &orig);

    void trim();

    [[nodiscard]] LyricTime getBegin() const; // 直接获取存储的 begin

    void setBegin(const LyricTime &time);

    [[nodiscard]] LyricTime getEnd() const; // 直接获取存储的 end

    void setEnd(const LyricTime &time);

    [[nodiscard]] LyricTime getDuration() const;

    [[nodiscard]] LyricTime getLineBegin() const; // 获取整行的 begin

    [[nodiscard]] LyricTime getLineEnd() const; // 获取整行的 end

    [[nodiscard]] LyricTime getLineDuration() const;

    [[nodiscard]] LyricTime getInnerBegin() const; // 获取第一个有效音节的 begin

    [[nodiscard]] LyricTime getInnerEnd() const; // 获取最后一个有效音节的 end

    [[nodiscard]] LyricTime getInnerDuration() const;

    [[nodiscard]] QString getAgent() const;

    [[nodiscard]] QString getKey() const;

    [[nodiscard]] bool isDuet() const;

    [[nodiscard]] bool haveBgLine() const;

    [[nodiscard]] std::shared_ptr<LyricLine> getBgLine() const;

    void removeRuby();

private:
    std::shared_ptr<LyricLine> _bg_line{};

    QList<std::shared_ptr<LyricSyl>> _syl_s{};

    LyricTime _begin{LyricTime::min()};

    LyricTime _end{LyricTime::max()};

    QString _agent{};

    QString _key{};

    bool _is_duet{false};

    bool _is_bg{false};

    /**
     * @brief 音译\n
     * @code { lang:string, line:ref<LyricTrans> } @endcode
     */
    std::map<QString, std::shared_ptr<LyricTrans>> _transliteration{};

    /**
     * @brief 翻译\n
     * @code { lang:string, line:ref<LyricTrans> } @endcode
     */
    std::map<QString, std::shared_ptr<LyricTrans>> _translation{};

    static int _spl_offset;
};


#endif //LYRIC_PARSER_LYRICLINE_H
