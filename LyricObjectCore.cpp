//
// Created by LEGION on 2025/12/18.
//

#include <ranges>

#include "LyricObject.h"

QList<std::tuple<QString, QString, QString>> presetMetas{
        {"musicName", "音乐名称", "%1"},
        {"artists", "音乐作者", "%1"},
        {"album", "音乐专辑名称", "%1"},
        {"ncmMusicId", "歌曲关联网易云音乐 ID", R"(https://music.163.com/#/song?id=%1)"},
        {"qqMusicId", "歌曲关联 QQ 音乐 ID", R"(https://y.qq.com/n/ryqq_v2/songDetail/%1)"},
        {"spotifyId", "歌曲关联 Spotify 音乐 ID", R"(https://open.spotify.com/track/%1)"},
        {"appleMusicId", "歌曲关联 Apple Music 音乐 ID", R"(https://music.apple.com/cn/song/%1)"},
        {"isrc", "歌曲关联 ISRC", "%1"}
};

QMap<QString, QStringList> LyricObject::getPresetMeta() {
    QMap<QString, QStringList> preset{};
    QStringList presetKey{"ttmlAuthorGithubLogin", "musicName", "artists", "album", "ncmMusicId", "qqMusicId", "spotifyId", "appleMusicId", "isrc"};

    for (const auto &[key, value]: this->_meta_data_s) {
        if (presetKey.contains(key))
            preset[key].push_back(value);
    }

    return preset;
}

QList<QPair<QString, QString>> LyricObject::getExtraMeta() {
    QList<QPair<QString, QString> > extra{};
    QStringList presetKey{"musicName", "artists", "album", "ncmMusicId", "qqMusicId", "spotifyId", "appleMusicId", "isrc", "ttmlAuthorGithub", "ttmlAuthorGithubLogin"};

    for (const auto &[key, value]: this->_meta_data_s) {
        if (!presetKey.contains(key))
            extra.push_back({key, value});
    }

    return extra;
}

QString LyricObject::toTXT() {
    std::span spanLines = this->_line_s;
    auto resultView = spanLines
                    | std::views::transform([](const LyricLine& line){
                        return line.toTXT();
                    });
    return QStringList(resultView.begin(), resultView.end()).join("\n");
}

void LyricObject::appendSubLine(const SubType role, const std::map<QString, std::shared_ptr<LyricTrans>> &trans_map, const QString& line_key) {
    if (role == SubType::Translation) {
        for (const auto &[lang, trans]: trans_map) {
            if (not this->_translation_s.contains(lang)) this->_translation_s[lang] = {false, {}};
            this->_translation_s[lang].second[line_key] = trans;
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

    langs.append(this->_translation_s.keys());
    langs.append(this->_transliteration_s.keys());

    return langs;
}

QList<QString> LyricObject::getTransLangs() const {
    return this->_translation_s.keys();
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
