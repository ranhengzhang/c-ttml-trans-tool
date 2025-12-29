//
// Created by LEGION on 2025/12/14.
//

#ifndef TTML_TOOL_LYRICLINE_H
#define TTML_TOOL_LYRICLINE_H

#include "LyricSyl.h"
#include "LyricTime.h"
#include "utils.h"

#include <utility>
#include <map>

#include <QDomElement>
#include <QList>

#include "LyricObject.h"

class LyricObject;

class LyricLine {
public:
    using Status = utils::Status;

    using SubType = utils::SubType;

    using LyricTrans = utils::LyricTrans;

    friend LyricLine utils::normalizeBrackets(LyricLine &line);

    friend void utils::trim(LyricLine &line);

    void setKey(const QString &key);

    [[nodiscard]] static std::pair<LyricLine, Status> fromTTML(const QDomElement &p, LyricLine *parent, LyricObject &obj);

    [[nodiscard]] QString toTTML() const;

    [[nodiscard]] QString toInnerTTML(bool xmlns = false) const;

    [[nodiscard]] QString toTXT() const;

    [[nodiscard]] QString toASS(const QString& role = "orig", LyricTime parent_begin = {-1}, LyricTime parent_end = {-1}) const;

    [[nodiscard]] std::pair<QString, QStringList> toLRC(const QString &extra);

    [[nodiscard]] QString toLRC() const;

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

    [[nodiscard]] LyricTime getBegin() const;

    void setBegin(const LyricTime &time);

    [[nodiscard]] LyricTime getEnd() const;

    void setEnd(const LyricTime &time);

    [[nodiscard]] LyricTime getDuration() const;

    [[nodiscard]] LyricTime getLineBegin() const;

    [[nodiscard]] LyricTime getLineEnd() const;

    [[nodiscard]] LyricTime getLineDuration() const;

    [[nodiscard]] LyricTime getInnerBegin() const;

    [[nodiscard]] LyricTime getInnerEnd() const;

    [[nodiscard]] LyricTime getInnerDuration() const;

    [[nodiscard]] QString getKey() const;

    [[nodiscard]] bool isDuet() const;

    [[nodiscard]] bool haveBgLine() const;

    [[nodiscard]] std::shared_ptr<LyricLine> getBgLine() const;

private:
    std::shared_ptr<LyricLine> _bg_line{};

    QList<std::shared_ptr<LyricSyl>> _syl_s{};

    LyricTime _begin{LyricTime::min()};

    LyricTime _end{LyricTime::max()};

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
};


#endif //TTML_TOOL_LYRICLINE_H
