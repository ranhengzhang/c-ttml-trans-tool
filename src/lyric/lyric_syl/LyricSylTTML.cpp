//
// Created by LEGION on 2025/12/17.
//

#include <QList>

#include "lyricsyl.hpp"

std::pair<LyricSyl, LyricSyl::Status> fromRuby(const QDomNode &span) {
    LyricSyl syl;
    const auto el = span.toElement();

    syl.setText(el.text());
    const auto begin = LyricTime::parse(el.attribute("begin"));
    if (!begin.second) return {{}, LyricSyl::Status::InvalidStructure};
    syl.setBegin(begin.first);
    const auto end = LyricTime::parse(el.attribute("end"));
    if (!end.second) return {{}, LyricSyl::Status::InvalidStructure};
    syl.setEnd(end.first);

    return {syl, LyricSyl::Status::Success};
}

std::pair<LyricSyl, LyricSyl::Status> LyricSyl::fromTTML(const QDomNode &span) {
    if (span.isText()) {
        return {fromText(span.nodeValue()), Status::Success};
    }

    LyricSyl syl;
    const auto el = span.toElement();

    if (el.isNull()) return {{}, Status::InvalidStructure};

    syl._is_text = false;
    if (el.hasAttribute("tts:ruby") and el.attribute("tts:ruby") == "container") {
        syl._is_phrase_start = el.hasAttribute("amll:rubyPhraseStart") and el.attribute("amll:rubyPhraseStart") == "true";
        const auto span_s = el.childNodes();
        for (int i = 0; i < span_s.count(); ++i) {
            // ReSharper disable once CppTooWideScopeInitStatement
            auto span = span_s.at(i);
            if (span.toElement().hasAttribute("tts:ruby")) {
                const auto ruby_role = span.toElement().attribute("tts:ruby");
                if (ruby_role == "base") {
                    syl._text = span.toElement().text();
                } elif (ruby_role == "textContainer") {
                    const auto ruby_s = span.childNodes();
                    for (int j = 0; j < ruby_s.count(); ++j) {
                        // ReSharper disable once CppTooWideScopeInitStatement
                        auto ruby = ruby_s.at(j).toElement();
                        if (ruby.hasAttribute("tts:ruby") and ruby.attribute("tts:ruby") == "text") {
                            auto [ruby_syl, success] = fromRuby(ruby);
                            if (success != Status::Success) return {{}, success};
                            syl._ruby_s.push_back(std::make_shared<LyricSyl>(std::move(ruby_syl)));
                        }
                    }
                    if (syl._ruby_s.empty()) return {{}, Status::InvalidStructure};
                    syl.setBegin(syl._ruby_s.front()->getBegin());
                    syl.setEnd(syl._ruby_s.back()->getEnd());
                }
            }
        }

        return {syl, Status::Success};
    } else {
        syl._is_explicit = el.hasAttribute("amll:obscene") and el.attribute("amll:obscene") == "true";
        const auto begin = LyricTime::parse(el.attribute("begin"));
        if (!begin.second) return {{}, Status::InvalidTimeFormat};
        syl._begin = begin.first;
        const auto end = LyricTime::parse(el.attribute("end"));
        if (!end.second) return {{}, Status::InvalidTimeFormat};
        syl._end = end.first;
        syl._text = el.text();

        return {syl, Status::Success};
    }
}

QString toRuby(const std::vector<std::shared_ptr<LyricSyl>> &ruby_s) {
    QStringList ret{};

    for (const auto &syl : ruby_s)
        ret.push_back(QString(R"(<span tts:ruby="text" begin="%1" end="%2">%3</span>)")
        .arg(syl->getBegin().toString(false, false, true))
        .arg(syl->getEnd().toString(false, false, true))
        .arg(lyric::utils::toHtmlEscaped(syl->getText())));

    return ret.join("");
}

QString LyricSyl::toTTML(bool xmlns) {
    return this->isText() ? lyric::utils::toHtmlEscaped(this->_text) :
    this->_ruby_s.empty() ? QString(R"(<span begin="%1" end="%2"%3%4>%5</span>)")
    .arg(this->_begin.toString(false,false,true))
    .arg(this->_end.toString(false,false,true))
    .arg(this->_is_explicit ? R"( amll:obscene="true")" : "")
    .arg(xmlns ? R"( xmlns="http://www.w3.org/ns/ttml")" : "")
    .arg(lyric::utils::toHtmlEscaped(this->_text))
    : QString(R"(<span tts:ruby="container"%1>%2%3</span>)")
    .arg(this->_is_phrase_start ? R"( amll:rubyPhraseStart="true")" : "")
    .arg(QString(R"(<span tts:ruby="base">%1</span>)").arg(lyric::utils::toHtmlEscaped(this->_text)))
    .arg(QString(R"(<span tts:ruby="textContainer">%1</span>)").arg(toRuby(this->_ruby_s)));
}

void LyricSyl::setOrig(const std::shared_ptr<LyricSyl> &orig_shared) {
    this->_orig = orig_shared;
    this->setBegin(orig_shared->getBegin());
    this->setEnd(orig_shared->getEnd());
}

std::shared_ptr<LyricSyl> LyricSyl::getOrig() const {
    return this->_orig.lock();
}
