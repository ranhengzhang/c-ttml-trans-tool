//
// Created by LEGION on 2025/12/18.
//

#include <ranges>

#include "LyricLine.hpp"
#include "LyricObject.hpp"

#include <QSet>

QMap<QString, QStringList> LyricObject::getPresetMeta() {
    QMap<QString, QStringList> preset{};
    const QStringList preset_key{"ttmlAuthorGithubLogin", "musicName", "artists", "album", "ncmMusicId", "qqMusicId", "spotifyId", "appleMusicId", "isrc"};

    for (const auto &[key, value]: this->_meta_data_s) {
        if (preset_key.contains(key))
            preset[key].push_back(value);
    }

    return preset;
}

QList<QPair<QString, QString>> LyricObject::getExtraMeta() {
    QList<QPair<QString, QString> > extra{};
    const QStringList preset_key{"musicName", "artists", "album", "ncmMusicId", "qqMusicId", "spotifyId", "appleMusicId", "isrc", "ttmlAuthorGithub", "ttmlAuthorGithubLogin"};

    for (const auto &[key, value]: this->_meta_data_s) {
        if (!preset_key.contains(key))
            extra.push_back({key, value});
    }

    return extra;
}

QStringList LyricObject::getSongWriter() {
    return this->_song_writer_s;
}

QString LyricObject::toTXT() {
    std::span span_lines = this->_line_s;
    auto result_view = span_lines
                    | std::views::transform([](const LyricLine& line){
                        return line.toTXT();
                    });
    return QStringList(result_view.begin(), result_view.end()).join("\n");
}

void LyricObject::appendSubLine(const SubType role, const std::map<QString, std::shared_ptr<LyricTrans>> &trans_map, const QString& line_key) {
    if (role == SubType::Translation) {
        for (const auto &[lang, trans]: trans_map) {
            this->_translation_s[{lang, false}][line_key] = trans;
        }
    } else {
        for (const auto &[lang, trans]: trans_map) {
            this->_transliteration_s[lang][line_key] = trans;
        }
    }
}

QString LyricObject::getTitle(const QString &postfix) {
    QString title = "";

    for (const auto &[key, value]: this->_meta_data_s) {
        if (key == "musicName") {
            title = value + (postfix.isEmpty() ? "" : ("." + postfix));
            break;
        }
    }

    return title;
}

QList<QString> LyricObject::getSubLangs() const {
    QList<QString> langs;

    langs.append(this->getTransLangs());
    langs.append(this->_transliteration_s.keys());

    return langs;
}

QList<QString> LyricObject::getTransLangs() const {
    auto filted_langs = this->_translation_s.keys()
    | std::views::transform([](const auto& p) { return p.first; }) // 提取 .first
    | std::views::common;

    // 转为 QSet 进行去重
    const QSet distinct_keys(filted_langs.begin(), filted_langs.end());

    return distinct_keys.values();
}

QList<QString> LyricObject::getRomaLangs() const {
    return this->_transliteration_s.keys();
}

QString LyricObject::getLang() const {
    return this->_lang;
}

LyricTime LyricObject::getDur() {
    LyricTime time{};

    for (const auto& line : this->_line_s) {
        time = std::max(time, line.getLineEnd());
    }

    return time;
}

bool LyricObject::haveDuet() const {
    return this->_have_duet;
}

bool LyricObject::haveBg() const {
    return this->_have_bg;
}

bool LyricObject::haveRoman() const {
    return this->_transliteration_s.count() > 0;
}

bool LyricObject::haveTrans() const {
    return this->_translation_s.count() > 0;
}

void LyricObject::removeRuby() {
    for (auto& line: this->_line_s) {
        line.removeRuby();
    }
}
