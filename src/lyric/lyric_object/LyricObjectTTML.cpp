//
// Created by LEGION on 2025/12/14.
//

#include "LyricObject.hpp"
#include "LyricLine.hpp"

#include <ranges>

#include <QString>
#include <QDomDocument>
#include <QRegularExpression>
#include <QSet>

QRegularExpression pairs_reg(R"([(（]+(.*?)[）)]+)");

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

    const auto agent_s = tt.elementsByTagName("ttm:agent");
    for (int i = 0; i < agent_s.length(); ++i) {
        auto xml = agent_s.at(i).toElement();
        auto [agent, status] = lyric::utils::Agent::fromTTML(xml);

        if (status == Status::InvalidFormat) return {{}, Status::InvalidFormat};
        lyric._agent_s.push_back(agent);
    }

    QString main_person_agent_id{};
    bool is_duet_person = false;

    QString main_group_agent_id{};
    bool is_duet_group = true;

    {
        std::unique_ptr<lyric::utils::Agent> main_person_agent{};
        std::unique_ptr<lyric::utils::Agent> main_group_agent{};

        for (auto &agent : lyric._agent_s) {
            if (agent.isPerson() and not main_person_agent) {
                main_person_agent = std::make_unique<lyric::utils::Agent>(agent);
                main_person_agent_id = agent.getId();
            } elif (agent.isGroup() and not main_group_agent) {
                main_group_agent = std::make_unique<lyric::utils::Agent>(agent);
                main_group_agent_id = agent.getId();
            }
        }
        lyric._have_duet = lyric._agent_s.length();
    }

    // region 元数据

    const auto iTunesMetadata = tt.elementsByTagName("iTunesMetadata");
    if (iTunesMetadata.length() > 0) {
        lyric._leading_silence = iTunesMetadata.at(0).toElement().attribute("leadingSilence");
    }

    const auto audio = tt.elementsByTagName("audio");
    if (audio.length() > 0) {
        lyric._lyric_offset = audio.at(0).toElement().attribute("lyricOffset");
    }

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
                QStringList en_key{"LYRIC", "LYRICS", "LYRICIST", "LYRICSAUTHOR", "LYRICSBY", "WORDS", "WORDSBY", "TEXTBY", "POEMBY", "COMPOSER"};
                QStringList cn_key = {"作词", "填词", "歌词"};
                if ((en_key.contains(key.toUpper().replace("_", "").replace("-", "").replace(" ", "")) or cn_key.contains(key)) and not lyric._song_writer_s.contains(value)) {
                    lyric._song_writer_s.push_back(value);
                }
            }
        }
    }

    // endregion

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
            auto line_agent_id = line.getAgent();
            auto line_agent = std::ranges::find_if(lyric._agent_s, [&line_agent_id](const auto &agent){return agent.getId() == line_agent_id;});
            if (line_agent == lyric._agent_s.end()) return {{}, Status::InvalidStructure};
            auto &agent = *line_agent;
            if (agent.isGroup()) {
                if (line_agent_id != main_group_agent_id) {
                    is_duet_group = !is_duet_group;
                    main_group_agent_id = line_agent_id;
                }
            } else {
                if (line_agent_id != main_person_agent_id) {
                    is_duet_person = !is_duet_person;
                    main_person_agent_id = line_agent_id;
                }
                line.setIsDuet(is_duet_person);
            }
            lyric._line_s.push_back(line);
            lyric._have_bg |= line.haveBgLine();
            lyric._have_duet |= line.isDuet();
        }
    }

    // region 音译
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
                if (!el_s.at(k).isText() && el_s.at(k).toElement().attribute("ttm:role") != "bg")
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
                    lyric::utils::normalizeBrackets(*sub_line.second);
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
    // endregion

    // region 翻译
    const auto translation_s = tt.elementsByTagName("translation");
    for (int i = 0; i < translation_s.length(); ++i) {
        const auto translation = translation_s.at(i).toElement();
        const auto lang = translation.attribute("xml:lang");
        const auto text_s = translation.elementsByTagName("text");
        const auto type = translation.attribute("type");

        if (type == "replacement") { // 逐字翻译
            auto &lang_translations = lyric._translation_s[{lang, true}];
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
                lang_translations[key] = ptr;
                orig_line.appendSubLine(SubType::Translation, lang, ptr);
            }
        } else { // 逐行翻译
            auto &lang_translations = lyric._translation_s[{lang, false}];
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
                    lyric::utils::normalizeBrackets(*sub_line.second);
                } elif (match.hasMatch()) {
                    sub_line.second = std::make_shared<QString>(match.captured(1));
                    sub_line.first.replace(match.capturedStart(0), match.capturedLength(0), "");
                }
                auto ptr = std::make_shared<LyricTrans>(sub_line);
                lang_translations[key] = ptr;
                orig_line.appendSubLine(SubType::Translation, lang, ptr);
            }
        }
    }
    // endregion

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

std::optional<QString> selectLang(QList<std::pair<QString, bool>> langs) {
    auto filted_langs = langs
    | std::views::transform([](const auto& p) { return p.first; }) // 提取 .first
    | std::views::common;

    // 转为 QSet 进行去重
    QSet distinct_keys(filted_langs.begin(), filted_langs.end());

    return selectLang(distinct_keys.values());
}

QString LyricObject::toTTML() {
    QStringList agent_data_list{};

    for (auto &agent: this->_agent_s) {
        agent_data_list.push_back(agent.toTTML());
    }

    auto agent_data_text = agent_data_list.join("");

    QStringList meta_data_list{};

    for (auto &[key, value]: this->_meta_data_s) {
        meta_data_list.push_back(QString(R"(<amll:meta key="%1" value="%2"/>)")
        .arg(lyric::utils::toHtmlEscaped(key))
        .arg(lyric::utils::toHtmlEscaped(value)));
    }

    auto meta_data_text = meta_data_list.join("");

    // region 翻译
    auto translation_text = QString();
    if (this->_translation_s.isEmpty()) {
        translation_text = "<translations/>";
    } else {
        translation_text = "<translations>";
        auto langs = this->_translation_s.keys();
        auto lang_opt = selectLang(langs);
        if (lang_opt) {
            const auto& lang = *lang_opt;
            if (langs.contains({lang, false})) {
                langs.removeAll({lang, false});
                langs.push_back({lang, false});
            }
            if (langs.contains({lang, true})) {
                langs.removeAll({lang, true});
                langs.push_back({lang, true});
            }
        }
        for (const auto& [lang, is_word]: langs) {
            const auto &translation_map = this->_translation_s[{lang, is_word}];

            translation_text += QString(R"(<translation type="%1" xml:lang="%2">)")
            .arg(is_word ? "replacement" : "subtitle")
            .arg(lang);
            // Sort keys by numeric value
            QStringList sorted_keys;
            for (const auto &target: translation_map | std::views::keys) {
                sorted_keys.push_back(target);
            }
            std::ranges::sort(sorted_keys, lyric::utils::compareKeysByLengthThenDefault);

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
                        .arg(lyric::utils::toHtmlEscaped(text.first))
                        .arg(text.second ? QString(R"( <span xmlns:ttm="http://www.w3.org/ns/ttml#metadata" ttm:role="x-bg" xmlns="http://www.w3.org/ns/ttml">(%1)</span>)").arg(lyric::utils::toHtmlEscaped(*text.second)) : "");
                    }
                }
            }
            translation_text += "</translation>";
        }
        translation_text += "</translations>";
    }
    // endregion

    // region 音译
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
            std::ranges::sort(sorted_keys, lyric::utils::compareKeysByLengthThenDefault);

            // Iterate through sorted keys
            for (const auto& target : sorted_keys) {
                const auto& content = transliteration_map.at(target);
                // 尝试获取 QString
                if (auto text = std::get_if<std::pair<QString,std::shared_ptr<QString>>>(content.get())) {
                    if (not text->first.isEmpty() or not text->second) {
                        transliteration_text += QString(R"(<text for="%1">%2</text>%3)")
                        .arg(target)
                        .arg(lyric::utils::toHtmlEscaped(text->first))
                        .arg(text->second ? QString(R"( <span xmlns:ttm="http://www.w3.org/ns/ttml#metadata" ttm:role="x-bg" xmlns="http://www.w3.org/ns/ttml">(%1)</span>)").arg(lyric::utils::toHtmlEscaped(*text->second)) : "");
                    }
                }
                // 尝试获取 LyricLine
                elif (auto line = std::get_if<LyricLine>(content.get())) {
                    transliteration_text += QString(R"(<text for="%1">%2</text>)").arg(target).arg(line->toInnerTTML(true));
                }
            }
            transliteration_text += "</transliteration>";
        }
        transliteration_text += "</transliterations>";
    }
    // endregion

    auto song_writer_text = QString();
    if (this->_song_writer_s.isEmpty()) {
        song_writer_text = R"(<songwriters/>)";
    } else {
        const auto song_writer_view = this->_song_writer_s | std::views::transform([](const auto &song_writer) {return QString(R"(<songwriter>%1</songwriter>)").arg(lyric::utils::toHtmlEscaped(song_writer));});
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

    return QString(R"(<tt xmlns="http://www.w3.org/ns/ttml" xmlns:amll="http://www.example.com/ns/amll" xmlns:itunes="http://music.apple.com/lyric-ttml-internal" xmlns:ttm="http://www.w3.org/ns/ttml#metadata" xmlns:tts="http://www.w3.org/ns/ttml#styling" itunes:timing="Word" xml:lang="%1"><head><metadata>%2%3%4</metadata></head><body dur="%5">%6</body></tt>)")
        .arg(this->_lang)
        .arg(agent_data_text)
        .arg(meta_data_text)
        .arg(!this->_translation_s.isEmpty() || !this->_transliteration_s.isEmpty() || !this->_song_writer_s.isEmpty() ?
            QString(R"(<iTunesMetadata xmlns="http://music.apple.com/lyric-ttml-internal"%1>%2%3</iTunesMetadata>)")
                .arg(this->_leading_silence.isEmpty() ?
                    "" :
                    QString(R"( leadingSilence="%1")")
                        .arg(this->_leading_silence))
                .arg(translation_text + song_writer_text + transliteration_text)
                .arg(this->_lyric_offset.isEmpty() ?
                    "" :
                    QString(R"(<audio lyricOffset="%1" role="spatial"/>)")
                        .arg(this->_leading_silence)) :
            "")
        .arg(this->getDur().toString(false, false, true))
        .arg(line_text)
        .replace(reg, " ")
        .trimmed();
}
