//
// Created by LEGION on 2025/12/14.
//

#include <QList>

#include "lyricline.hpp"
#include "../lyric_object/LyricObject.hpp"

void LyricLine::setKey(const QString &key) {
    _key = key;
}

std::pair<LyricLine, LyricLine::Status> LyricLine::fromTTML(const QDomElement &p, LyricLine *parent, LyricObject &obj) {
    LyricLine line;

    line._is_bg = parent != nullptr;
    line._is_duet = parent ? parent->_is_duet : "v1" != p.attribute("ttm:agent");
    if (parent == nullptr && p.tagName() == "p") {
        const auto begin = LyricTime::parse(p.attribute("begin"));
        if (!begin.second) return {{}, Status::InvalidTimeFormat};
        line._begin = begin.first;
        const auto end = LyricTime::parse(p.attribute("end"));
        if (!end.second) return {{}, Status::InvalidTimeFormat};
        line._end = end.first;
        line._key = p.attribute(p.tagName() == "text" ? "for" : "itunes:key");
    }

    const auto span_s = p.childNodes();
    for (int i = 0; i < span_s.count(); ++i) {
        // ReSharper disable once CppTooWideScopeInitStatement
        auto span = span_s.at(i);
        if (span.toElement().hasAttribute("ttm:role")) {
            const auto role = span.toElement().attribute("ttm:role");
            if (role == "x-bg") {
                auto [bg_line, status] = fromTTML(span.toElement(), &line, obj);
                if (status != Status::Success) return {{}, status};
                line._bg_line = std::make_shared<LyricLine>(bg_line);
            } else {
                auto text = span.firstChild().nodeValue();
                auto lang = QString();
                if (span.toElement().hasAttribute("xml:lang")) lang = span.toElement().attribute("xml:lang");
                else lang = obj.getLang() + "-Latn";
                if (role == "x-translation") {
                    if (line._is_bg) {
                        utils::normalizeBrackets(text);
                        std::get<std::pair<QString,std::shared_ptr<QString>>>(*parent->_translation[lang]).second = std::make_shared<QString>(text);
                        line._translation[lang] = std::make_shared<LyricTrans>(text);
                    } else {
                        line._translation[lang] = std::make_shared<LyricTrans>(std::pair<QString, std::shared_ptr<QString>>{text, {}});
                    }
                } elif (role == "x-roman") {
                    if (line._is_bg) {
                        utils::normalizeBrackets(text);
                        std::get<std::pair<QString,std::shared_ptr<QString>>>(*parent->_transliteration[lang]).second = std::make_shared<QString>(text);
                        line._transliteration[lang] = std::make_shared<LyricTrans>(text);
                    } else {
                        line._transliteration[lang] = std::make_shared<LyricTrans>(std::pair<QString, std::shared_ptr<QString>>{text, {}});
                    }
                }
            }
        } else {
            auto [syl, success] = LyricSyl::fromTTML(span);
            if (success != Status::Success) return {{}, success};
            line._syl_s.push_back(std::make_shared<LyricSyl>(std::move(syl)));
        }
    }
    if (line._syl_s.isEmpty()) {
        line._syl_s.push_back(std::make_shared<LyricSyl>(LyricSyl::fromText("")));
    } elif (line._syl_s.length() > 1) {
        // Process text-type syllables: set begin time to previous syllable's end time, end time to next syllable's begin time
        for (int i = 0; i < line._syl_s.length(); ++i) {
            auto& syl = line._syl_s[i];
            if (syl->getIsText()) {
                // Set begin time to previous syllable's end time (or line begin if first syllable)
                if (i > 0) {
                    syl->setBegin(line._syl_s[i-1]->getEnd());
                } else {
                    syl->setBegin(line._begin);
                }

                // Set end time to next syllable's begin time (or line end if last syllable)
                if (i < line._syl_s.length() - 1) {
                    syl->setEnd(line._syl_s[i+1]->getBegin());
                } else {
                    syl->setEnd(line._end);
                }
            }
        }
    }
    if (line._is_bg) {
        utils::normalizeBrackets(line);
    } else {
        if (not line._translation.empty()) {
            obj.appendSubLine(SubType::Translation, line._translation, line._key);
        }
        if (not line._transliteration.empty()) {
            obj.appendSubLine(SubType::Transliteration, line._transliteration, line._key);
        }
    }

    return {line, Status::Success};
}

QString LyricLine::toTTML() const {
    return QString(R"(<p begin="%1" end="%2" itunes:key="%3" ttm:agent="v%4">%5</p>)")
    .arg(this->getLineBegin().toString(false, false, true))
    .arg(this->getLineEnd().toString(false, false, true))
    .arg(this->_key)
    .arg(this->_is_duet ? "2" : "1")
    .arg(this->toInnerTTML());
}

QString LyricLine::toInnerTTML(const bool xmlns) const {
    QStringList text;
    for (const auto &syl : this->_syl_s) text.push_back(syl->toTTML(xmlns));
    if (this->_is_bg) {
        if (text.count()) {
            if (text.first().contains('>'))
                text.first().insert(text.first().indexOf('>') + 1, '(');
            else
                text.first().push_front('(');

            if (text.last().contains('<'))
                text.last().insert(text.last().lastIndexOf('<'), ')');
            else
                text.last().push_back(')');
        }
    }
    auto ret = text.join("");
    if (this->_bg_line)
        ret += QString(R"( <span ttm:role="x-bg"%1>%2</span>)")
        .arg(xmlns ? R"( xmlns:ttm="http://www.w3.org/ns/ttml#metadata" xmlns="http://www.w3.org/ns/ttml")" : "")
        .arg(this->_bg_line->toInnerTTML());
    return ret;
}
