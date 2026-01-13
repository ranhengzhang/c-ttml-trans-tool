//
// Created by LEGION on 2025/12/14.
//

#include "LyricObject.h"

#include <ranges>

#include <QString>
#include <QDomDocument>
#include <QRegularExpression>

QRegularExpression pairs_reg(R"([(（]+(.*?)[）)]+)");

// Helper function to extract numeric part from key (e.g., "L10" -> 10)
int extractNumberFromKey(const QString &key) {
    QRegularExpression numReg(R"(\d+)");
    auto match = numReg.match(key);
    if (match.hasMatch()) {
        return match.captured(0).toInt();
    }
    return 0;
}

// Comparator for keys based on numeric value
bool compareKeysByNumber(const QString &key1, const QString &key2) {
    return extractNumberFromKey(key1) < extractNumberFromKey(key2);
}

std::pair<LyricObject, LyricObject::Status> LyricObject::fromTTML(const QString &ttml) {
    QDomDocument doc;
    const auto res = doc.setContent(ttml.trimmed(), QDomDocument::ParseOption::PreserveSpacingOnlyNodes);
    if (!res) return {{}, Status::InvalidFormat};

    const auto tt = doc.documentElement();
    if (tt.elementsByTagName("head").length() != 1 ||
        tt.elementsByTagName("body").length() != 1 ||
        tt.elementsByTagName("div").length() == 0 ||
        tt.elementsByTagName("p").length() == 0) {
        return {{}, Status::InvalidStructure};
    }

    LyricObject lyric{};
    if (tt.hasAttribute("xml:lang")) lyric._lang = tt.attribute("xml:lang");
    else lyric._lang = "zh-Hans";
    lyric._have_duet = tt.elementsByTagName("ttm:agent").length() > 1;

    const auto song_writer_s = tt.elementsByTagName("songwriter");
    for (int i = 0; i < song_writer_s.length(); ++i) {
        auto song_writer = song_writer_s.at(i).toElement();
        lyric._song_writer_s.push_back(song_writer.text());
    }

    const auto meta_data_s = tt.elementsByTagName("amll:meta");
    for (int i = 0; i < meta_data_s.length(); ++i) {
        auto meta_data = meta_data_s.at(i).toElement();
        const auto key = meta_data.attribute("key");
        // ReSharper disable once CppTooWideScopeInitStatement
        const auto value = meta_data.attribute("value");
        if (key.toUpper() == "SONGWRITER") {
            if (not lyric._song_writer_s.contains(value)) lyric._song_writer_s.push_back(value);
        } else {
            if (not lyric._meta_data_s.contains({key, value})) {
                lyric._meta_data_s.push_back({key, value});
                if ((key.toUpper() == "LYRIC" or key.toUpper() == "LYRICS" or key == "作词") and not lyric._song_writer_s.contains(value)) {
                    lyric._song_writer_s.push_back(value);
                }
            }
        }
    }

    const auto div_s = tt.elementsByTagName("div");
    for (int i = 0; i < div_s.length(); ++i) {
        lyric._song_part_s.push_back({});

        auto div = div_s.at(i).toElement();
        auto p_s = div.elementsByTagName("p");

        if (div.hasAttribute("itunes:songPart")) lyric._song_part_s.back().song_part = div.attribute("itunes:songPart");
        elif (div.hasAttribute("itunes:song-part")) lyric._song_part_s.back().song_part = div.attribute("itunes:song-part");
        lyric._song_part_s.back().count = p_s.count();

        for (int j = 0; j < p_s.length(); ++j) {
            const auto p = p_s.at(j).toElement();
            auto [line, status] = LyricLine::fromTTML(p, nullptr, lyric);

            if (status != Status::Success) return {{}, status};
            if (line.getKey().isEmpty()) line.setKey(QString("L%1").arg(lyric._line_s.length() + 1));
            line.trim();
            lyric._line_s.push_back(line);
            lyric._have_bg |= line.haveBgLine();
            lyric._have_duet |= line.isDuet();
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
            const auto key = text.attribute("for");
            int count = 0;

            for (int k = 0; k < el_s.length(); ++k) {
                if (!text_s.at(k).isText() && text_s.at(k).toElement().attribute("ttm:role") != "bg")
                    ++count;
            }

            auto orig_ptr = std::ranges::find_if(lyric._line_s, [&key](const auto &line){return line.getKey() == key;});
            if (orig_ptr == lyric._line_s.end()) continue;
            auto &orig_line = *orig_ptr;
            if (count == 0) { // 逐行音译
                std::pair<QString,std::shared_ptr<QString>> sub_line = {text.childNodes().at(0).nodeValue(),{}};
                auto match = pairs_reg.match(sub_line.first);
                if (text.childNodes().length() > 1) {
                    sub_line.second = std::make_shared<QString>(text.childNodes().at(1).childNodes().at(0).nodeValue());
                    utils::normalizeBrackets(*sub_line.second);
                } elif (match.hasMatch()) {
                    sub_line.second = std::make_shared<QString>(match.captured(1));
                    sub_line.first.replace(match.capturedStart(0), match.capturedLength(0), "");
                }
                auto ptr = std::make_shared<LyricTrans>(sub_line);
                lyric._transliteration_s[lang][key] = ptr;
                orig_line.appendSubLine(SubType::Transliteration, lang, ptr);
            } else { // 逐字音译
                auto [sub_line, status] = LyricLine::fromTTML(text, nullptr, lyric);
                if (status != Status::Success) return {{}, status};
                sub_line.match(orig_line);
                auto ptr = std::make_shared<LyricTrans>(sub_line);
                lyric._transliteration_s[lang][key] = ptr;
                orig_line.appendSubLine(SubType::Transliteration, lang, ptr);
            }
        }
    }

    const auto translation_s = tt.elementsByTagName("translation");
    for (int i = 0; i < translation_s.length(); ++i) {
        const auto translation = translation_s.at(i).toElement();
        const auto lang = translation.attribute("xml:lang");
        const auto text_s = translation.elementsByTagName("text");
        const auto type = translation.attribute("type");

        if (type == "replacement") { // 逐字翻译
            lyric._translation_s[lang].first = true;
            for (int j = 0; j < text_s.length(); ++j) {
                const auto text = text_s.at(j).toElement();
                const auto key = text.attribute("for");
                auto orig_ptr = std::ranges::find_if(lyric._line_s, [&key](const auto &line){return line.getKey() == key;});;
                if (orig_ptr == lyric._line_s.end()) continue;
                auto &orig_line = *orig_ptr;
                auto [sub_line, status] = LyricLine::fromTTML(text, nullptr, lyric);
                if (status != Status::Success) return {{}, status};
                sub_line.match(orig_line);
                auto ptr = std::make_shared<LyricTrans>(sub_line);
                if (not lyric._translation_s.contains(lang)) lyric._translation_s[lang] = {true, {}};
                lyric._translation_s[lang].second[key] = ptr;
                orig_line.appendSubLine(SubType::Translation, lang, ptr);
            }
        } else { // 逐行翻译
            lyric._translation_s[lang].first = false;
            for (int j = 0; j < text_s.length(); ++j) {
                const auto text = text_s.at(j).toElement();
                const auto key = text.attribute("for");
                auto orig_ptr = std::ranges::find_if(lyric._line_s, [&key](const auto &line){return line.getKey() == key;});
                if (orig_ptr == lyric._line_s.end()) continue;
                auto &orig_line = *orig_ptr;
                std::pair<QString, std::shared_ptr<QString>> sub_line = {text.childNodes().at(0).nodeValue(),{}};
                auto match = pairs_reg.match(sub_line.first);
                if (text.childNodes().length() > 1) {
                    sub_line.second = std::make_shared<QString>(text.childNodes().at(1).childNodes().at(0).nodeValue());
                    utils::normalizeBrackets(*sub_line.second);
                } elif (match.hasMatch()) {
                    sub_line.second = std::make_shared<QString>(match.captured(1));
                    sub_line.first.replace(match.capturedStart(0), match.capturedLength(0), "");
                }
                auto ptr = std::make_shared<LyricTrans>(sub_line);
                if (not lyric._translation_s.contains(lang)) lyric._translation_s[lang] = {false, {}};
                lyric._translation_s[lang].second[key] = ptr;
                orig_line.appendSubLine(SubType::Translation, lang, ptr);
            }
        }
    }

    return {lyric, Status::Success};
}

std::optional<QString> selectLang(QList<QString> langs) {
    if (langs.contains("zh-Hans")) return "zh-Hans";
    if (langs.contains("zh-CN")) return "zh-CN";
    if (langs.contains("zh-Hant")) return "zh-Hant";
    auto zh_key = std::ranges::find_if(langs, [](const auto &key){return key.startsWith("zh");});
    if (zh_key != langs.end()) return *zh_key;
    return langs.isEmpty() ? std::nullopt : std::optional(*langs.begin());
}

QString LyricObject::toTTML() {
    const auto meta_data_view = this->_meta_data_s | std::views::transform([](const auto &meta_data) {return QString(R"(<amll:meta key="%1" value="%2"/>)").arg(utils::toHtmlEscaped(meta_data.key)).arg(utils::toHtmlEscaped(meta_data.value));});
    const auto meta_data_text = QStringList(meta_data_view.begin(), meta_data_view.end()).join("");

    auto translation_text = QString();
    if (this->_translation_s.isEmpty()) {
        translation_text = "<translations/>";
    } else {
        translation_text = "<translations>";
        auto langs = this->_translation_s.keys();
        auto lang_opt = selectLang(langs);
        if (lang_opt) {
            const auto& lang = *lang_opt;
            langs.removeAll(lang);
            langs.push_back(lang);
        }
        for (const auto& lang: langs) {
            const auto &[is_word, translation_map] = this->_translation_s[lang];

            translation_text += QString(R"(<translation type="%1" xml:lang="%2">)")
            .arg(is_word ? "replacement" : "subtitle")
            .arg(lang);
            // Sort keys by numeric value
            QStringList sorted_keys;
            for (const auto &target: translation_map | std::views::keys) {
                sorted_keys.push_back(target);
            }
            std::ranges::sort(sorted_keys, compareKeysByNumber);

            // Iterate through sorted keys
            for (const auto& target : sorted_keys) {
                const auto& content = translation_map.at(target);
                if (is_word) {
                    translation_text += QString(R"(<text for="%1">%2</text>)").arg(target).arg(std::get<LyricLine>(*content).toInnerTTML(true));
                } else {
                    auto text = std::get<std::pair<QString,std::shared_ptr<QString>>>(*content);
                    if (not text.first.isEmpty() or not text.second) {
                        translation_text += QString(R"(<text for="%1">%2%3</text>)")
                        .arg(target)
                        .arg(text.first)
                        .arg(text.second ? QString(R"( <span xmlns:ttm="http://www.w3.org/ns/ttml#metadata" ttm:role="x-bg" xmlns="http://www.w3.org/ns/ttml">(%1)</span>)").arg(*text.second) : "");
                    }
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
            const auto &transliteration_map = this->_transliteration_s[lang];

            // Sort keys by numeric value
            QStringList sorted_keys;
            for (const auto &target: transliteration_map | std::views::keys) {
                sorted_keys.push_back(target);
            }
            std::ranges::sort(sorted_keys, compareKeysByNumber);

            // Iterate through sorted keys
            for (const auto& target : sorted_keys) {
                const auto& content = transliteration_map.at(target);
                // 尝试获取 QString
                if (auto text = std::get_if<std::pair<QString,std::shared_ptr<QString>>>(content.get())) {
                    if (not text->first.isEmpty() or not text->second) {
                        transliteration_text += QString(R"(<text for="%1">%2</text>%3)")
                        .arg(target)
                        .arg(text->first)
                        .arg(text->second ? QString(R"( <span xmlns:ttm="http://www.w3.org/ns/ttml#metadata" ttm:role="x-bg" xmlns="http://www.w3.org/ns/ttml">(%1)</span>)").arg(*text->second) : "");
                    }
                }
                // 尝试获取 LyricLine
                elif (auto* line = std::get_if<LyricLine>(content.get())) {
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
        const auto song_writer_view = this->_song_writer_s | std::views::transform([](const auto &song_writer) {return QString(R"(<songwriter>%1</songwriter>)").arg(utils::toHtmlEscaped(song_writer));});
        song_writer_text = QString(R"(<songwriters>%1</songwriters>)").arg(QStringList(song_writer_view.begin(), song_writer_view.end()).join(""));
    }

    int index = 0;
    auto line_text = QString();
    for (const auto&[count, songPart] : this->_song_part_s) {
        line_text += QString(R"(<div begin="%1" end="%2"%3>)")
        .arg(this->_line_s.at(index).getLineBegin().toString(false, false, true))
        .arg(this->_line_s.at(index+count - 1).getLineEnd().toString(false, false, true))
        .arg(songPart.length() || this->_song_part_s.length() > 1 ? QString(R"( itunes:songPart="%1")").arg(songPart.length() ? songPart : "Verse") : "");
        for (int i = 0; i < count; ++i) {
            line_text += this->_line_s.at(index++).toTTML();
        }
        line_text += "</div>";
    }

    const auto reg = QRegularExpression(R"(\s{2,})");

    return QString(R"(<tt xmlns="http://www.w3.org/ns/ttml" xmlns:amll="http://www.example.com/ns/amll" xmlns:itunes="http://music.apple.com/lyric-ttml-internal" xmlns:ttm="http://www.w3.org/ns/ttml#metadata" itunes:timing="Word" xml:lang="%1"><head><metadata><ttm:agent type="person" xml:id="v1"/>%2%3%4</metadata></head><body dur="%5">%6</body></tt>)")
    .arg(this->_lang)
    .arg(this->_have_duet ? R"(<ttm:agent type="other" xml:id="v2"/>)" : "")
    .arg(meta_data_text)
    .arg(!this->_translation_s.isEmpty() || !this->_transliteration_s.isEmpty() || !this->_song_writer_s.isEmpty() ? QString(R"(<iTunesMetadata xmlns="http://music.apple.com/lyric-ttml-internal">%1</iTunesMetadata>)").arg(translation_text + song_writer_text + transliteration_text) : "")
    .arg(this->getDur().toString(false, false, true))
    .arg(line_text)
    .replace(reg, " ")
    .trimmed();
}
