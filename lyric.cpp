//
// Created by LEGION on 25-4-27.
//

#include "lyric.h"

#include <qdatetime.h>
#include <QMessageBox>
#include <QScreen>

Lyric Lyric::parse(const QDomElement &tt, bool *ok) {
    Lyric lyric{};
    const auto head_s = tt.elementsByTagName(R"(head)");
    const auto body_s = tt.elementsByTagName(R"(body)");

    if (head_s.length() != 1 || body_s.length() != 1) {
        *ok = false;
        return {};
    }

    const auto head = head_s.at(0).toElement();
    const auto body = body_s.at(0).toElement();

    const auto metadata_s = head.elementsByTagName(R"(metadata)");

    if (metadata_s.length() != 1) {
        *ok = false;
        return {};
    }

    const auto metadata = metadata_s.at(0).toElement();
    const auto meta_s = metadata.childNodes();

    // 解析元数据信息
    for (int i = 0; i < meta_s.length(); ++i) {
        // ReSharper disable once CppTooWideScopeInitStatement
        auto child = meta_s.at(i).toElement();
        if (child.tagName() == R"(ttm:agent)") {
            lyric._have_duet |= child.attribute(R"(xml:id)") != R"(v1)";
        } else if (child.tagName() == R"(amll:meta)") {
            auto key = child.attribute(R"(key)");
            auto value = child.attribute(R"(value)");

            if (key.isEmpty() || value.isEmpty()) {
                *ok = false;
                return {};
            }
            lyric._meta_s.push_back({key, value});
        }
    }

    const auto div_s = body.elementsByTagName(R"(div)");

    if (div_s.length() != 1) {
        *ok = false;
        return {};
    }

    const auto div = div_s.at(0).toElement();
    const auto p_s = div.childNodes();

    // 解析歌词行
    for (int i = 0; i < p_s.length(); ++i) {
        const auto p = p_s.at(i).toElement();
        auto line = LyricLine::parse(p, false, ok);

        if (!*ok) {
            return {};
        }
        lyric._line_s.push_back(line);
        lyric._have_bg |= line.haveBg();
        lyric._have_roman |= line.haveRoman();
        lyric._lang_s = lyric._lang_s.unite(line.getLang());
        lyric._end = line.getEnd();
    }

    *ok = true;
    return lyric;
}

// ReSharper disable once CppDFAConstantFunctionResult
QString Lyric::getTitle(const QString &postfix) {
    QString title = "";

    for (const auto &[key, value]: _meta_s) {
        if (key == R"(musicName)") {
            title = value + (postfix.isEmpty() ? "" : ("." + postfix));
            break;
        }
    }

    return title;
}

QString Lyric::toTTML() {
    return QString(
                R"(<tt xmlns="http://www.w3.org/ns/ttml" xmlns:ttm="http://www.w3.org/ns/ttml#metadata" xmlns:amll="http://www.example.com/ns/amll" xmlns:itunes="http://music.apple.com/lyric-ttml-internal">%1%2</tt>)")
            .arg(this->getTTMLHead())
            .arg(this->getTTMLBody());
}

QString Lyric::toASS() {
    return this->getASSHead() + this->getASSBody();
}

QString Lyric::toLRC(const QString &extra) {
    return this->getLRCHead() + "\n\n" + this->getLRCBody(extra);
}

QString Lyric::toSPL() {
    return this->getLRCHead() + "\n\n" + this->getSPLBody();
}

QPair<QString, QString> Lyric::toLYS(const QString &lang) {
    auto lys = this->getLYSBody(lang);

    lys.first.insert(0, this->getLRCHead() + "\n\n");

    return lys;
}

QMap<QString, QString> Lyric::toQRC(const QString &lang) {
    auto qrc = this->getQRCBody(lang);

    qrc[R"(orig)"].insert(0, this->getLRCHead() + "\n\n");
    if (!qrc[R"(ts)"].isEmpty())
        qrc[R"(ts)"].insert(0, this->getLRCHead() + "\n\n");
    if (!qrc[R"(roman)"].isEmpty())
        qrc[R"(roman)"].insert(0, this->getLRCHead() + "\n\n");

    return qrc;
}

QPair<QString, QString> Lyric::toYRC(const QString &lang) {
    QStringList orig{};
    QStringList ts{};

    for (auto &line: this->_line_s) {
        auto [orig_yrc, ts_yrc] = line.toYRC(lang);
        orig.append(orig_yrc);
        if (!ts_yrc.isEmpty())
            ts.append(ts_yrc);
    }

    return {orig.join("\n"), ts.join("\n")};
}

QString Lyric::toTXT() {
    QStringList text{};
    for (auto &line: this->_line_s)
        text.append(line.toTXT());
    return text.join("\n");
}

bool Lyric::haveDuet() const {
    return this->_have_duet;
}

bool Lyric::haveBg() const {
    return this->_have_bg;
}

bool Lyric::haveRoman() const {
    return this->_have_roman;
}

QList<QString> Lyric::getLangs() const {
    return this->_lang_s.values();
}

QString Lyric::getTTMLHead() {
    QStringList text{};

    text.append(R"(<head><metadata>)");
    text.append(R"(<ttm:agent type="person" xml:id="v1"/>)");
    if (this->_have_duet)
        text.append(R"(<ttm:agent type="other" xml:id="v2"/>)");
    for (const auto &[key, value]: _meta_s)
        text.append(QString(R"(<amll:meta key="%1" value="%2"/>)").arg(key, value));
    text.append(R"(</metadata></head>)");

    return text.join("");
}

QString Lyric::getTTMLBody() {
    QStringList text{};

    text.append(QString(R"(<body dur="%1">)").arg(this->_line_s.back().getEnd().toString(false, false, true)));
    text.append(QString(R"(<div begin="%1" end="%2">)")
        .arg(this->_line_s.front().getBegin().toString(false, false, true))
        .arg(this->_line_s.back().getEnd().toString(false, false, true)));
    for (int i = 0; i < this->_line_s.size(); ++i)
        text.append(this->_line_s[i].toTTML(i + 1));
    text.append(R"(</div></body>)");

    return text.join("");
}

QString Lyric::getASSHead() {
    QSize size(1920, 1080);
    // ReSharper disable once CppTooWideScope
    const auto screen = QGuiApplication::primaryScreen();

    if (screen) {
        size = screen->size();
    }

    return QString(R"([Script Info]
; Script generated by Aegisub 3.2.2
; http://www.aegisub.org/
Title: %1
ScriptType: v4.00+
Timer: 100.0000
PlayResX: %2
PlayResY: %3
WrapStyle: 0
ScaledBorderAndShadow: no

[Aegisub Project Garbage]
Last Style Storage: Default

[V4+ Styles]
Format: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding
Style: orig-furigana,Arial,72,&H0000FF&,&H000000&,&HFFFFFF&,&H00000000,0,0,0,0,100,100,6,0,1,5,0,2,10,10,10,128
Style: orig,Arial,128,&H0000FF&,&H000000&,&HFFFFFF&,&H00000000,0,0,0,0,100,100,7.5,0,1,6.5,0,2,10,10,10,1
Style: roma,Arial,52,&H0000FF&,&H000000&,&HFFFFFF&,&H00000000,0,0,0,0,100,100,0,0,1,5,0,2,10,10,255,0
Style: ts,Arial,94,&H0000FF&,&H000000&,&HFFFFFF&,&H00000000,0,0,0,0,100,100,0,0,1,6.5,0,8,10,10,10,1

[Events]
Format: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text
)").arg(this->getTitle()).arg(size.width()).arg(size.height());
}

QString Lyric::getASSBody() {
    QStringList text{};

    for (auto &line: this->_line_s)
        text.append(line.toASS());

    return text.join("\n");
}

QString Lyric::getLRCHead() {
    QMap<QString, QStringList> meta;

    for (const auto &[key, value]: _meta_s) {
        if (key == R"(musicName)") {
            meta[R"(ti)"].append(value);
        } else if (key == R"(artists)") {
            meta[R"(ar)"].append(value);
        } else if (key == R"(album)") {
            meta[R"(al)"].append(value);
        }
    }

    QStringList text{};

    for (const auto& value: meta[R"(ti)"])
        text.append(QString(R"([ti:%1])").arg(value));
    for (const auto& value: meta[R"(ar)"])
        text.append(QString(R"([ar:%1])").arg(value));
    for (const auto& value: meta[R"(al)"])
        text.append(QString(R"([al:%1])").arg(value));

    return text.join("\n");
}

QString Lyric::getLRCBody(const QString &extra) {
    QStringList text{};

    text.append(R"([00:00.00])");
    for (int i = 0; i < this->_line_s.size(); ++i)
        text.append(
            this->_line_s[i].toLRC(extra, i < this->_line_s.size() - 1 ? this->_line_s[i + 1].getBegin() : this->_end));
    text.append(QString(R"([%1])").arg(this->_end.toString(false, true, true)));

    return text.join("\n");
}

QString Lyric::getSPLBody() {
    QStringList text{};

    for (auto &line: this->_line_s)
        text.append(line.toSPL());

    return text.join("\n");
}

QPair<QString, QString> Lyric::getLYSBody(const QString &lang) {
    QStringList orig{};
    QStringList ts{};

    for (auto &line: this->_line_s) {
        auto [lys_orig, lys_ts] = line.toLYS(this->haveBg(), this->haveDuet(), lang);
        orig.append(lys_orig);
        if (!lys_ts.isEmpty())
            ts.append(lys_ts);
    }

    return {orig.join("\n"), ts.join("\n")};
}

QMap<QString, QString> Lyric::getQRCBody(const QString &lang) {
    QStringList orig{};
    QStringList ts{};
    QStringList roman{};

    for (auto &line: this->_line_s) {
        auto qrc = line.toQRC(lang);
        orig.append(qrc[R"(orig)"]);
        if (!qrc[R"(ts)"].isEmpty())
            ts.append(qrc[R"(ts)"]);
        if (!qrc[R"(roman)"].isEmpty())
            roman.append(qrc[R"(roman)"]);
    }

    return {{R"(orig)", orig.join("\n")}, {R"(ts)", ts.join("\n")}, {R"(roman)", roman.join("\n")}};
}
