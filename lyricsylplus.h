//
// Created by LEGION on 2025/11/21.
//

#ifndef TTMLX_LYRICSYLPLUS_H
#define TTMLX_LYRICSYLPLUS_H

#include <QDomElement>

#include "lyrictimeplus.h"


class LyricSylPlus {
public:
    [[nodiscard]] static LyricSylPlus parse(const QDomNode &span);

    [[nodiscard]] static LyricSylPlus parse(const QString &text);

    [[nodiscard]] LyricTimePlus getDuration() const;

    [[nodiscard]] QString getText() const;

    void setText(const QString &text);

    [[nodiscard]] bool getIsText() const;

    [[nodiscard]] bool isText() const;

    void setIsText(bool is_text);

    [[nodiscard]] LyricTimePlus getBegin() const;

    [[nodiscard]] LyricTimePlus getEnd() const;

    void setOrig(const std::shared_ptr<LyricSylPlus>& orig_shared);

    [[nodiscard]] std::shared_ptr<LyricSylPlus> getOrig() const;

    [[nodiscard]] QString toTTML(bool xmlns = false);

private:

    QString _text{};
    LyricTimePlus _begin{};
    LyricTimePlus _end{};
    std::weak_ptr<LyricSylPlus> _orig{};
    bool _is_text{};
};


#endif //TTMLX_LYRICSYLPLUS_H
