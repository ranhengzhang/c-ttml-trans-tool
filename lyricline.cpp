//
// Created by LEGION on 25-4-27.
//

#include "lyricline.h"

LyricLine LyricLine::parse(const QDomElement &p, const bool is_bg, bool *ok) {
    // NOLINT(*-no-recursion)
    LyricLine line;

    const auto begin = p.attribute(R"(begin)");
    const auto end = p.attribute(R"(end)");
    const auto agent = p.attribute(R"(ttm:agent)");

    line._is_bg = is_bg;
    line._is_duet = R"(v1)" != agent;
    line._begin = LyricTime::parse(begin, ok);
    if (!ok) return {};
    line._end = LyricTime::parse(end, ok);
    // ReSharper disable once CppDFAConstantConditions
    // ReSharper disable once CppDFAUnreachableCode
    if (!ok) return {};

    const auto syl_s = p.childNodes();

    LyricTime last_end = line._begin;

    for (int i = 0; i < syl_s.count(); ++i) {
        const auto child = syl_s.at(i);
        if (child.isText()) {
            LyricTime next_begin;
            if (i != syl_s.count() - 1) {
                next_begin = LyricTime::parse(syl_s.at(i + 1).toElement().attribute(R"(begin)"), ok);
            } else {
                next_begin = line._end;
            }
            auto syl = LyricSyl::parse(child.nodeValue(), last_end, next_begin, ok);
            // ReSharper disable once CppDFAConstantConditions
            // ReSharper disable once CppDFAUnreachableCode
            if (ok) line._syl_s.push_back(syl);
            continue;
        }

        const auto element = child.toElement();
        // ReSharper disable once CppTooWideScopeInitStatement
        const auto role = element.attribute(R"(ttm:role)");

        if (role.isEmpty()) {
            // normal syl
            auto syl = LyricSyl::parse(element, ok);
            line._syl_s.push_back(syl);
            // ReSharper disable once CppDFAConstantConditions
            // ReSharper disable once CppDFAUnreachableCode
            if (!ok) return {};
            last_end = syl.getEnd();
        } else if (R"(x-bg)" == role) {
            // bg line
            auto bg_line = LyricLine::parse(element, true, ok);
            // ReSharper disable once CppDFAConstantConditions
            // ReSharper disable once CppDFAUnreachableCode
            if (!ok) return {};
            line._bg_line = std::make_unique<LyricLine>(std::move(bg_line));
        } else if (R"(x-translation)" == role) {
            // trans line
            QString lang = element.attribute(R"(xml:lang)");

            line._trans.insert(lang, element.firstChild().nodeValue());
        } else if (R"(x-roman)" == role) {
            // roman line
            line._roman = element.text();
        }
    }

    *ok = true;
    return line;
}

bool LyricLine::haveBg() const {
    return this->_bg_line.operator bool();
}

bool LyricLine::haveRoman() const {
    return !this->_roman.isEmpty();
}

LyricTime LyricLine::getBegin() const {
    return this->_begin;
}

LyricTime LyricLine::getEnd() const {
    return this->_end;
}

QSet<QString> LyricLine::getLang() const {
    auto lang = this->_trans.keys();
    return QSet(lang.begin(), lang.end());
}

QString LyricLine::toTTML(const int num) { // NOLINT(misc-no-recursion)
    QStringList line{};

    line.append(QString(R"(<%1 begin="%2" end="%3" %4>)")
        .arg(this->_is_bg ? R"(span)" : R"(p)")
        .arg(this->_begin.toString(false, false, true))
        .arg(this->_end.toString(false, false, true))
        .arg(this->_is_bg
                 ? R"(ttm:role="x-bg")"
                 : QString(R"(ttm:agent="%1" itunes:key="L%2")")
                 .arg(this->_is_duet ? R"(v2)" : R"(v1)")
                 .arg(num)));

    for (auto &syl: this->_syl_s)
        line.append(syl.toTTML());
    if (!this->_trans.isEmpty())
        for (auto lang: this->_trans.keys())
            line.append(QString(R"(<span ttm:role="x-translation" xml:lang="%1">%2</span>)")
                .arg(lang)
                .arg(this->_trans[lang]));
    if (!this->_roman.isEmpty())
        line.append(QString(R"(<span ttm:role="x-roman">%1</span>)").arg(this->_roman));
    if (this->_bg_line)
        line.append(this->_bg_line->toTTML());
    line.append(this->_is_bg ? R"(</span>)" : R"(</p>)");

    return line.join("");
}

QString LyricLine::toASS() {
    QStringList line{};

    line.append(R"(Dialogue: 0)");
    line.append(this->_begin.toString(true, true, true));
    line.append(this->_end.toString(true, true, true));
    line.append(R"(orig)");
    line.append(QString(R"(%1 %2)")
        .arg(this->_is_bg ? R"(x-bg)" : "")
        .arg(this->_is_duet ? R"(x-duet)" : "")
        .trimmed());
    line.append(R"(0,0,0,karaoke)");

    QStringList text{};
    LyricTime last = this->_begin;

    for (int i = 0; i < this->_syl_s.size(); ++i) {
        auto syl = this->_syl_s[i];

        if (syl.getBegin() > last)
            text.append(QString(R"({\k%1})").arg((syl.getBegin() - last) / 10));
        if (i == 0)
            syl.normalize(LyricSyl::NormalizeOption::FRONT);
        if (i == this->_syl_s.size() - 1)
            syl.normalize(LyricSyl::NormalizeOption::BACK);
        if (i < this->_syl_s.size() - 1) {
            // ReSharper disable once CppTooWideScopeInitStatement
            auto &next = this->_syl_s[i + 1];
            if (syl.getEnd() > next.getBegin())
                syl.setEnd(next.getBegin());
        }
        text.append(syl.toASS());
        last = syl.getEnd();
    }
    line.append(text.join(""));

    QStringList ass{};
    ass.append(line.join(","));
    if (!this->_roman.isEmpty())
        ass.append(QString(R"(Dialogue: 0,%1,%2,roma,%3,0,0,0,,%4)")
            .arg(this->_begin.toString(true, true, true))
            .arg(this->_end.toString(true, true, true))
            .arg(this->_is_bg ? R"(x-bg)" : "")
            .arg(this->_roman));
    if (!this->_trans.isEmpty()) {
        for (const auto& lang: this->_trans.keys())
            ass.append(QString(R"(Dialogue: 0,%1,%2,ts,%3,0,0,0,,%4)")
                .arg(this->_begin.toString(true, true, true))
                .arg(this->_end.toString(true, true, true))
                .arg(QString(R"(%1 %2)")
                    .arg(this->_is_bg ? R"(x-bg)" : "")
                    .arg(R"(x-lang:)" + lang)
                    .trimmed())
                .arg(this->_trans[lang]));
    }
    if (this->_bg_line)
        ass.append(this->_bg_line->toASS());

    return ass.join("\n");
}

QString LyricLine::toTXT() {
    QStringList line{};

    for (auto &syl: this->_syl_s)
        line.append(syl.getText());

    return line.join("");
}

QString LyricLine::toLRC(const QString &extra, const LyricTime next) {
    QStringList line{};
    QString ext{};

    line.append(QString(R"([%1]%2)")
        .arg(this->_begin.toString(false, true, true))
        .arg(this->toTXT()));
    if (extra.startsWith(R"(x-roman)") && !this->_roman.isEmpty())
        ext = this->_roman;
    else if (extra.startsWith(R"(x-trans)")) {
        // ReSharper disable once CppTooWideScopeInitStatement
        const auto lang = extra.split(":").last();
        if (this->_trans.contains(lang))
            ext = this->_trans[lang];
    }
    if (!ext.isEmpty())
        line.append(QString(R"([%1]%2)")
            .arg(next.toString(false, true, true))
            .arg(ext));
    if (this->_bg_line) {
        line.append(QString(R"([%1]%2)")
            .arg(next.toString(false, true, true))
            .arg(this->_bg_line->toTXT()));
        if (extra.startsWith(R"(x-roman)") && !this->_bg_line->_roman.isEmpty()) {
            line.append(QString(R"([%1](%2))")
                .arg(next.toString(false, true, true))
                .arg(this->_bg_line->_roman));
        } else if (extra.startsWith(R"(x-trans)")) {
            // ReSharper disable once CppTooWideScopeInitStatement
            const auto lang = extra.split(":").last();
            if (this->_bg_line->_trans.contains(lang))
                line.append(QString(R"([%1](%2))")
                    .arg(next.toString(false, true, true))
                    .arg(this->_bg_line->_trans[lang]));
        }
    }
    if (next - this->_end >= (5 * 1000))
        line.append(QString(R"([%1])").arg(this->_end.toString(false, true, true)));

    return line.join("\n");
}

QString LyricLine::toSPL() {
    QStringList line{};
    auto last = this->_begin;

    line.append(QString(R"([%1])").arg(this->_begin.toString(false, false, true)));
    for (auto &syl: this->_syl_s) {
        if (syl.getBegin() > last)
            line.append(QString(R"(<%1>)").arg(last.toString(false, false, true)));
        line.append(syl.toSPL());
        last = syl.getEnd();
    }
    line.append(QString(R"(<%1>)").arg(this->_syl_s.back().getEnd().toString(false, false, true)));
    line.append(QString(R"([%1])").arg(this->_end.toString(false, false, true)));

    QStringList text{};

    text.append(line.join(""));
    if (!this->_trans.isEmpty())
        for (auto lang: this->_trans.keys())
            text.append(this->_trans[lang]);
    if (!this->_roman.isEmpty())
        text.append(this->_roman);
    if (this->_bg_line) {
        text.append(this->_bg_line->toTXT());
        if (!this->_bg_line->_trans.isEmpty())
            for (auto lang: this->_bg_line->_trans.keys())
                text.append("(" + this->_bg_line->_trans[lang] + ")");
        if (!this->_bg_line->_roman.isEmpty())
            text.append("(" + this->_bg_line->_roman + ")");
    }

    return text.join("\n");
}

QPair<QString, QString> LyricLine::toLYS(const bool have_bg, const bool have_duet, const QString &lang) {
    QStringList line{};

    line.append(QString(R"([%1])").arg((have_bg + this->_is_bg) * 3 + (have_duet + this->_is_duet)));
    for (auto &syl: this->_syl_s)
        line.append(syl.toQRC());

    QStringList orig{};
    QStringList ts{};

    orig.append(line.join(""));
    if (this->_trans.contains(lang)) {
        if (this->_trans.contains(lang))
            ts.append(QString(R"([%1]%2)")
                .arg(this->_begin.toString(false, false, true))
                .arg(this->_trans[lang]));
    }
    if (this->_bg_line) {
        const auto [bg_orig, bg_ts] = this->_bg_line->toLYS(have_bg, have_duet, lang);
        orig.append(bg_orig);
        if (!bg_ts.isEmpty())
            ts.append(bg_ts);
    }

    return {orig.join("\n"), ts.join("\n")};
}

QMap<QString, QString> LyricLine::toQRC(const QString &lang) {
    QStringList line{};
    const auto start = this->_begin.getCount();
    const auto dur = this->_end - this->_begin;

    line.append(QString(R"([%1,%2])")
        .arg(start)
        .arg(dur));
    for (auto &syl: this->_syl_s)
        line.append(syl.toQRC());

    QStringList orig{};
    QStringList ts{};
    QStringList roman{};

    orig.append(line.join(""));
    if (this->_trans.contains(lang))
        ts.append(QString(R"([%1]%2)")
            .arg(this->_begin.toString(false, true, true))
            .arg(this->_trans[lang]));
    if (!this->_roman.isEmpty())
        roman.append(QString(R"([%1,%2]%3)")
            .arg(this->_begin.getCount())
            .arg(this->_end - this->_begin)
            .arg(this->_roman));
    if (this->_bg_line) {
        const auto bg = this->_bg_line->toQRC(lang);
        orig.append(bg[R"(orig)"]);
        if (!bg[R"(ts)"].isEmpty())
            ts.append(bg[R"(ts)"]);
        if (!bg[R"(roman)"].isEmpty())
            roman.append(bg[R"(roman)"]);
    }

    return {{R"(orig)", orig.join("")}, {R"(ts)", ts.join("")}, {R"(roman)", roman.join("")}};
}

QPair<QString, QString> LyricLine::toYRC(const QString &lang) {
    QStringList line{};

    line.append(QString(R"([%1,%2])")
        .arg(this->_begin.getCount())
        .arg(this->_end - this->_begin));
    for (auto &syl: this->_syl_s)
        line.append(syl.toYRC());

    QStringList orig{};
    QStringList ts{};

    orig.append(line.join(""));
    if (this->_trans.contains(lang))
        ts.append(QString(R"([%1]%2)")
            .arg(this->_begin.toString(false, true, true))
            .arg(this->_trans[lang]));
    if (this->_bg_line) {
        const auto [bg_orig, bg_ts] = this->_bg_line->toYRC(lang);
        orig.append(bg_orig);
        if (!bg_ts.isEmpty())
            ts.append(bg_ts);
    }

    return {orig.join("\n"), ts.join("\n")};
}
