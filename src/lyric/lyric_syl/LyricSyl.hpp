//
// Created by LEGION on 2025/12/14.
//

#ifndef TTML_TOOL_LYRICSYL_H
#define TTML_TOOL_LYRICSYL_H

#include "../lyric_time/LyricTime.hpp"
#include "../utils/utils.hpp"

#include <QString>
#include <QDomDocument>

class LyricSyl {
    friend bool operator==(const LyricSyl &lhs, const LyricSyl &rhs) {
        return lhs._text == rhs._text
               && lhs._begin == rhs._begin
               && lhs._end == rhs._end
               && lhs._orig.lock() == rhs._orig.lock()
               && lhs._is_text == rhs._is_text;
    }

public:
    using Status = utils::Status;

    [[nodiscard]] static std::pair<LyricSyl, Status> fromTTML(const QDomNode &span);

    [[nodiscard]] QString toTTML(bool xmlns = false);

    [[nodiscard]] QString toASS() const;

    [[nodiscard]] QString toSPL() const;

    [[nodiscard]] QString toQRC() const;

    [[nodiscard]] QString toYRC() const;

    [[nodiscard]] QString toKRC(const LyricTime &line_begin) const;

    void setOrig(const std::shared_ptr<LyricSyl>& orig_shared);

    [[nodiscard]] std::shared_ptr<LyricSyl> getOrig() const;

    [[nodiscard]] static LyricSyl fromText(const QString &text);

    [[nodiscard]] LyricTime getDuration() const;

    [[nodiscard]] QString getText() const;

    void setText(const QString &text);

    [[nodiscard]] bool getIsText() const;

    [[nodiscard]] bool isText() const;

    void setIsText(bool is_text);

    [[nodiscard]] LyricTime getBegin() const;

    void setBegin(const LyricTime &begin);

    [[nodiscard]] LyricTime getEnd() const;

    void setEnd(const LyricTime &end);

    [[nodiscard]] bool getIsExplicit() const;

private:
    QString _text{};
    LyricTime _begin{};
    LyricTime _end{};
    std::weak_ptr<LyricSyl> _orig{};
    bool _is_text{};
    bool _is_explicit{};
};


#endif //TTML_TOOL_LYRICSYL_H
