//
// Created by LEGION on 2025/12/10.
//

#include <QRegularExpression>
#include <QXmlStreamReader>

#include "utils.hpp"
#include "lyricsyl.hpp"
#include "lyricline.hpp"

using Qt::Literals::StringLiterals::operator""_L1;

QRegularExpression before_reg(R"(^[\(（]?)");
QRegularExpression after_reg(R"([）\)]?$)");

QString &lyric::utils::easyCompress(QString &text) {
    text = text.trimmed()
    .replace(R"(" />)", R"("/>)")
    .replace(R"(" >)", R"(">)")
    .replace(R"(< )", R"(<)");

    const QRegularExpression compress_reg(R"([\n\r]+\s*)");
    text.replace(compress_reg, "");

    return text;
}

QString lyric::utils::toHtmlEscaped(const QString &text) {
    const auto pos = std::u16string_view(text).find_first_of(u"<>&\"'");
    if (pos == std::u16string_view::npos)
        return text;
    QString rich;
    const qsizetype len = text.size();
    rich.reserve(static_cast<qsizetype>(len * 1.1));
    rich += qToStringViewIgnoringNull(text).first(pos);
    for (auto ch : qToStringViewIgnoringNull(text).sliced(pos)) {
        if (ch == u'<')
            rich += "&lt;"_L1;
        elif (ch == u'>')
            rich += "&gt;"_L1;
        elif (ch == u'&')
            rich += "&amp;"_L1;
        elif (ch == u'"')
            rich += "&quot;"_L1;
        elif (ch == u'\'')
            rich += "&apos;"_L1;
        else
            rich += ch;
    }
    rich.squeeze();
    return rich;
}

QString lyric::utils::normalizeBrackets(QString &text) {
    // replace syls.front: _before_reg->'('
    const auto front_match = before_reg.match(text);
    if (front_match.hasMatch()) text = text.replace(front_match.capturedStart(), front_match.capturedLength(), "");
    // replace syls.back: _after_reg->')'
    const auto back_match = after_reg.match(text);
    if (back_match.hasMatch()) text = text.replace(back_match.capturedStart(), back_match.capturedLength(), "");

    return text;
}

LyricLine lyric::utils::normalizeBrackets(LyricLine &line) {
    // replace syls.front: _before_reg->'('
    const auto front_match = before_reg.match(line._syl_s.first()->getText());
    if (front_match.hasMatch()) {
        line._syl_s.first()->setText(line._syl_s.first()->getText().replace(front_match.capturedStart(), front_match.capturedLength(), ""));
    }
    // replace syls.back: _after_reg->')'
    const auto back_match = after_reg.match(line._syl_s.last()->getText());
    if (back_match.hasMatch()) {
        line._syl_s.last()->setText(line._syl_s.last()->getText().replace(back_match.capturedStart(), back_match.capturedLength(), ""));
    }

    return line;
}

QString lyric::utils::getDeepInnerText(const QDomNode &node) {
    QString result;
    const QDomNodeList children = node.childNodes();

    for (int i = 0; i < children.count(); ++i) {
        QDomNode child = children.at(i);

        if (child.isText() || child.isCDATASection()) {
            // 如果是文本节点，直接取值
            result += child.toText().data();
        } else if (child.isElement()) {
            // 如果是元素节点，递归获取其内部文本
            result += getDeepInnerText(child);
        }
    }
    return result;
}

QString lyric::utils::toLineTrans(QString &dom_text, const SubType line_type) {
    easyCompress(dom_text);

    QXmlStreamReader reader(dom_text);
    int depth = 0;
    int bg_depth = -1;
    int text_depth = -1;
    QString lang;
    QString key;
    std::pair<QString, QString> line{};
    QString *target = &line.first;
    QStringList buffer{};
    QRegularExpression re(R"(^[(（]*(.*?)[）)]*$)");

    while (not reader.atEnd() and not reader.hasError()) {
        if (reader.isStartElement()) {
            ++depth;
            if (reader.name() == u"translation" or reader.name() == u"transliteration") {
                lang = reader.attributes().value(u"xml:lang").toString();
            } elif (reader.name() == u"text") {
                text_depth = depth;
                line = {};
                key = reader.attributes().value(u"for").toString();
            } elif (reader.name() == u"span") {
                if (reader.attributes().value(u"ttm:role").toString() == u"x-bg") {
                    bg_depth = depth;
                    target = &line.second;
                }
            }
        } elif (reader.isEndElement()) {
            if (depth == bg_depth) {
                bg_depth = -1;
                target = &line.first;
            } elif (depth == text_depth) {
                if (not line.second.isEmpty()) {
                    if (re.match(line.second.trimmed()).hasMatch()) {
                        line.second = re.match(line.second.trimmed()).captured(1);
                    }
                }

                text_depth = -1;
                buffer.append(QString(R"(<text for="%1">%2%3</text>)")
                    .arg(key)
                    .arg(line.first.trimmed())
                    .arg(line.second.isEmpty() ? "" : QString(R"( <span xmlns:ttm="http://www.w3.org/ns/ttml#metadata" ttm:role="x-bg" xmlns="http://www.w3.org/ns/ttml">(%1)</span>)").arg(line.second.trimmed())));
            }
            --depth;
        } elif (reader.isCharacters()) {
            if (depth >= text_depth) target->append(reader.text().toString());
        }
        reader.readNext();
    }

    return QString(R"(<%1 %2>%3</%1>)")
        .arg(line_type == SubType::Transliteration ? "transliteration" : "translation")
        .arg((line_type == SubType::Transliteration ? QString(R"(xml:lang="%1")") : QString(R"(type="subtitle" xml:lang="%1")")).arg(lang))
        .arg(buffer.join(""));
}

static const QRegularExpression punct_regex(R"(^((?!&|＆)\p{P})+$)");

bool lyric::utils::isSymbol(const QString &text) {
    const auto trimmed = text.trimmed();

    if (trimmed.isEmpty()) return false;
    return punct_regex.match(trimmed).hasMatch();
}

bool lyric::utils::compareKeysByLengthThenDefault(const QString &key1, const QString &key2) {
    return key1.length() != key2.length() ? key1.length() < key2.length() : key1 < key2;
}
