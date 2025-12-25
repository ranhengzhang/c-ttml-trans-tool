//
// Created by LEGION on 2025/12/21.
//

#include "LyricObject.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

QString LyricObject::toKRC(const QString &ts_lang, const QString &roma_lang) {
    QStringList text{};
    QStringList sub_text = {};

    if (this->_translation_s.contains(ts_lang)) sub_text += this->getSubKRC(SubType::Translation, ts_lang);
    if (this->_transliteration_s.contains(roma_lang)) sub_text += this->getSubKRC(SubType::Transliteration, roma_lang);

    sub_text += R"(],"version":1})";

    // Base64编码
    const QByteArray base64 = QString(R"({"content":[%1],"version":1})").arg(sub_text.join(",")).toUtf8().toBase64();

    QString creator = "unknow";

    for (const auto &[key, value]: this->_meta_data_s) {
        if (key == R"(ttmlAuthorGithubLogin)") {
            creator = value;
            break;
        }
    }

    for (auto &orig_line: this->_line_s) text.push_back(orig_line.toKRC());

    return this->getLRCHead()
           + "[id:$00000000]\n"
           + "[sign:]\n"
           + "[qq:00000000\n]"
           + "[offset:0]\n"
           + QString("[by:%1]\n").arg(creator)
           + QString("[total:%1]\n").arg(static_cast<int64_t>(this->getDur()))
           + QString("[language:%1]\n").arg(QString::fromLatin1(base64))
           + text.join("\n");
}

QString LyricObject::getSubKRC(const SubType role, const QString &lang) {
    QStringList content{};

    for (auto &line: this->_line_s) {
        content.push_back(QString(R"([%1])").arg(line.getSubKRC(role, lang)));
    }

    return QString(R"({"language":0,"lyricContent":[%1],"type":%2})").arg(content.join(",")).arg(role == SubType::Translation ? 1 : 0);
}
