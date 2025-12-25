//
// Created by LEGION on 2025/12/17.
//

#include "LyricSyl.h"

LyricSyl LyricSyl::fromText(const QString &text) {
    LyricSyl syl;
    syl._is_text = true;
    syl._text = text;
    return syl;
}

LyricTime LyricSyl::getDuration() const {
    return this->_end - this->_begin;
}

void LyricSyl::setText(const QString &text) {
    this->_text = text;
}

QString LyricSyl::getText() const {
    return this->_text;
}

void LyricSyl::setIsText(bool is_text) {
    this->_is_text = is_text;
}

bool LyricSyl::getIsText() const {
    return this->_is_text;
}

bool LyricSyl::isText() const {
    auto origPtr = this->_orig.lock();

    return origPtr ? origPtr->isText() : (this->_is_text || this->_text.trimmed().length() == 0);
}

LyricTime LyricSyl::getBegin() const {
    return this->_begin;
}

void LyricSyl::setBegin(const LyricTime &begin) {
    this->_begin = begin;
}

LyricTime LyricSyl::getEnd() const {
    return this->_end;
}

void LyricSyl::setEnd(const LyricTime &end) {
    this->_end = end;
}
