//
// Created by LEGION on 2025/11/18.
//

#include <QRegularExpression>
#include <ranges>

#include "lyricplus.h"

#include <iso646.h>

std::pair<LyricPlus, bool> LyricPlus::parse(const QDomElement &tt) {
    if (tt.elementsByTagName("head").length() != 1 ||
        tt.elementsByTagName("body").length() != 1 ||
        tt.elementsByTagName("div").length() == 0 ||
        tt.elementsByTagName("p").length() == 0) {
        return {{}, false};
    }

    LyricPlus lyric{};

    lyric.lang = tt.attribute("xml:lang");
    lyric._have_other = tt.elementsByTagName("ttm:agent").length() > 1;

    const auto song_writer_s = tt.elementsByTagName("songwriter");
    for (int i = 0; i < song_writer_s.length(); ++i) {
        auto song_writer = song_writer_s.at(i).toElement();
        lyric._song_writer_s.push_back(song_writer.text());
    }

    const auto meta_data_s = tt.elementsByTagName("amll:meta");
    for (int i = 0; i < meta_data_s.length(); ++i) {
        auto meta_data = meta_data_s.at(i).toElement();
        const auto key = meta_data.attribute("key");
        const auto value = meta_data.attribute("value");
        if (key.toUpper() == "SONGWRITER" and not lyric._song_writer_s.contains(value)) {
            lyric._song_writer_s.push_back(value);
        } else {
            lyric._meta_data_s.push_back({key, value});
            if ((key.toUpper() == "LYRIC" or key.toUpper() == "LYRICS" or key == "作词") and not lyric._song_writer_s.contains(value)) {
                lyric._song_writer_s.push_back(value);
            }
        }
    }

    const auto div_s = tt.elementsByTagName("div");
    for (int i = 0; i < div_s.length(); ++i) {
        lyric._song_part_s.push_back({});

        auto div = div_s.at(i).toElement();
        auto p_s = div.elementsByTagName("p");

        lyric._song_part_s.back().songPart = div.attribute("itunes:songPart");
        lyric._song_part_s.back().count = p_s.count();

        for (int j = 0; j < p_s.length(); ++j) {
            const auto p = p_s.at(j).toElement();
            auto [line, success] = LyricLinePlus::parse(p, nullptr);

            if (!success) return {{}, false};

            lyric._line_s.push_back(line);
        }
    }

    const auto transliteration_s = tt.elementsByTagName("transliteration");
    for (int i = 0; i < transliteration_s.length(); ++i) {
        const auto transliteration = transliteration_s.at(i).toElement();
        const auto lang = transliteration.attribute("xml:lang");
        const auto text_s = transliteration.elementsByTagName("text");

        for (int j = 0; j < text_s.length(); ++j) {
            const auto text = text_s.at(j).toElement();
            const auto el_s = text.childNodes();
            int count = 0;

            for (int k = 0; k < el_s.length(); ++k) {
                if (!text_s.at(k).isText() && text_s.at(k).toElement().attribute("ttm:role") != "bg")
                    ++count;
            }

            if (count == 0) { // 逐行音译
                std::pair<QString,QString> content = {text.childNodes().at(0).nodeValue(),{}};
                if (text.childNodes().length() > 1) {
                    content.second = text.childNodes().at(1).childNodes().at(0).nodeValue();
                }
                lyric._transliteration_s[lang].push_back({text.attribute("for"), content});
            } else { // 逐字音译
                auto [line, success] = LyricLinePlus::parse(text, nullptr);
                if (!success) return {{}, false};
                const auto key = text.attribute("for");
                const auto index = key.sliced(1).toInt() - 1;
                line.match(lyric._line_s.at(index));
                lyric._transliteration_s[lang].push_back({key, line});
            }
        }
    }

    const auto translation_s = tt.elementsByTagName("translation");
    for (int i = 0; i < translation_s.length(); ++i) {
        const auto translation = translation_s.at(i).toElement();
        const auto lang = translation.attribute("xml:lang");
        const auto text_s = translation.elementsByTagName("text");
        const auto type = translation.attribute("type");

        if (type == "replacement") { // 逐字音译
            lyric._translation_s[lang].first = true;
            for (int j = 0; j < text_s.length(); ++j) {
                auto [line, success] = LyricLinePlus::parse(text_s.at(j).toElement(), nullptr);
                if (!success) return {{}, false};
                const auto key = text_s.at(j).toElement().attribute("for");
                const auto index = key.sliced(1).toInt() - 1;
                line.match(lyric._line_s.at(index));
                lyric._translation_s[lang].second.push_back({text_s.at(j).toElement().attribute("for"), line});
            }
        } else { // 逐行翻译
            lyric._translation_s[lang].first = false;
            for (int j = 0; j < text_s.length(); ++j) {
                const auto text = text_s.at(j).toElement();
                std::pair<QString, QString> content = {text.childNodes().at(0).nodeValue(),{}};
                if (text.childNodes().length() > 1) {
                    content.second = text.childNodes().at(1).childNodes().at(0).nodeValue();
                }
                lyric._translation_s[lang].second.push_back({text.attribute("for"), content});
            }
        }
    }

    return {lyric, true};
}

QString LyricPlus::toTTML() {
    const auto meta_data_view = this->_meta_data_s | std::views::transform([](const auto &meta_data) {return QString(R"(<amll:meta key="%1" value="%2"/>)").arg(meta_data.key).arg(meta_data.value);});
    const auto meta_data_text = QStringList(meta_data_view.begin(), meta_data_view.end()).join("");

    auto translation_text = QString();
    if (this->_translation_s.isEmpty()) {
        translation_text = "<translations/>";
    } else {
        translation_text = "<translations>";
        for (const auto& lang : this->_translation_s.keys()) {
            const auto &[is_word, translation_list] = this->_translation_s[lang];

            translation_text += QString(R"(<translation type="%1" xml:lang="%2">)")
            .arg(is_word ? "replacement" : "subtitle")
            .arg(lang);
            for (const auto& [target, content] : translation_list) {
                if (is_word) {
                    translation_text += QString(R"(<text for="%1">%2</text>)").arg(target).arg(std::get<LyricLinePlus>(content).toInnerTTML(true));
                } else {
                    auto text = std::get<std::pair<QString,QString>>(content);
                    translation_text += QString(R"(<text for="%1">%2%3</text>)")
                    .arg(target)
                    .arg(text.first)
                    .arg(text.second.isEmpty() ? "" : QString(R"( <span xmlns:ttm="http://www.w3.org/ns/ttml#metadata" ttm:role="x-bg" xmlns="http://www.w3.org/ns/ttml">%1</span>)").arg(text.second));
                }
            }
            translation_text += "</translation>";
        }
        translation_text += "</translations>";
    }

    auto transliteration_text = QString();
    if (this->_transliteration_s.isEmpty()) {
        transliteration_text = "<transliterations/>";
    } else {
        transliteration_text = "<transliterations>";
        for (const auto& lang : this->_transliteration_s.keys()) {
            transliteration_text += QString(R"(<transliteration xml:lang="%1">)").arg(lang);
            const auto &transliteration_list = this->_transliteration_s[lang];

            for (const auto& [target, content] : transliteration_list) {
                // 尝试获取 QString
                if (auto* text = std::get_if<std::pair<QString,QString>>(&content)) {
                    transliteration_text += QString(R"(<text for="%1">%2</text>%3)")
                    .arg(target)
                    .arg(text->first)
                    .arg(text->second.isEmpty() ? "" : QString(R"( <span xmlns:ttm="http://www.w3.org/ns/ttml#metadata" ttm:role="x-bg" xmlns="http://www.w3.org/ns/ttml">%1</span>)").arg(text->second));
                }
                // 尝试获取 LyricLinePlus
                else if (auto* line = std::get_if<LyricLinePlus>(&content)) {
                    transliteration_text += QString(R"(<text for="%1">%2</text>)").arg(target).arg(line->toInnerTTML(true));
                }
            }
            transliteration_text += "</transliteration>";
        }
        transliteration_text += "</transliterations>";
    }

    auto song_writer_text = QString();
    if (this->_song_writer_s.isEmpty()) {
        song_writer_text = R"(<songwriters/>)";
    } else {
        const auto song_writer_view = this->_song_writer_s | std::views::transform([](const auto &song_writer) {return QString(R"(<songwriter>%1</songwriter>)").arg(song_writer);});
        song_writer_text = QString(R"(<songwriters>%1</songwriters>)").arg(QStringList(song_writer_view.begin(), song_writer_view.end()).join(""));
    }

    int index = 0;
    auto line_text = QString();
    for (const auto&[count, songPart] : this->_song_part_s) {
        line_text += QString(R"(<div begin="%1" end="%2"%3>)")
        .arg(this->_line_s.at(index).getBegin().toString(false, false, true))
        .arg(this->_line_s.at(index+count - 1).getEnd().toString(false, false, true))
        .arg(songPart.length() || this->_song_part_s.length() > 1 ? QString(R"( itunes:songPart="%1")").arg(songPart.length() ? songPart : "Verse") : "");
        for (int i = 0; i < count; ++i) {
            line_text += this->_line_s.at(index).toTTML();
            ++index;
        }
        line_text += "</div>";
    }

    const auto reg = QRegularExpression(R"(\s{2,})");

    return QString(R"(<tt xmlns="http://www.w3.org/ns/ttml" xmlns:amll="http://www.example.com/ns/amll" xmlns:itunes="http://music.apple.com/lyric-ttml-internal" xmlns:ttm="http://www.w3.org/ns/ttml#metadata" itunes:timing="Word" xml:lang="%1"><head><metadata><ttm:agent type="person" xml:id="v1"/>%2%3%4</metadata></head><body dur="%5">%6</body></tt>)")
    .arg(this->lang)
    .arg(this->_have_other ? R"(<ttm:agent type="other" xml:id="v2"/>)" : "")
    .arg(meta_data_text)
    .arg(!this->_translation_s.isEmpty() || !this->_transliteration_s.isEmpty() || !this->_song_writer_s.isEmpty() ? QString(R"(<iTunesMetadata xmlns="http://music.apple.com/lyric-ttml-internal">%1</iTunesMetadata>)").arg(translation_text + song_writer_text + transliteration_text) : "")
    .arg(this->_line_s.last().getEnd().toString(false, false, true))
    .arg(line_text)
    .replace(reg, " ")
    .trimmed();
}
