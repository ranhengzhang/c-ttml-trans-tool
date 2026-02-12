//
// Created by LEGION on 2026/2/12.
//

#include <QDebug>

#include "utils.hpp"

tool::utils::OpenCCConverter::OpenCCConverter(const BuiltinConfig config): _handle(nullptr), _is_valid(false) {
    // 1. 创建 OpenCC 实例
    const OpenCCResult result = opencc_create(config, &_handle);
    if (result == OpenCCResult::Success && _handle != nullptr) {
        _is_valid = true;
    } else {
        qWarning() << "Failed to create OpenCC instance. Error code:" << static_cast<int>(result);
    }
}

tool::utils::OpenCCConverter::~OpenCCConverter() {
    if (_handle != nullptr) {
        opencc_destroy(_handle);
    }
}

QString tool::utils::OpenCCConverter::convert(const QString &input_text) const {
    if (!_is_valid) {
        qWarning() << "OpenCCConverter is not valid, returning original text.";
        return input_text;
    }

    // 将 QString 转换为 UTF-8 编码的 C 字符串
    const QByteArray text_bytes = input_text.toUtf8();

    // 2. 调用 FFI 函数进行转换
    // ReSharper disable once CppTooWideScope
    char *converted_text = opencc_convert(_handle, text_bytes.constData());

    if (converted_text) {
        // 从返回的 C 字符串创建 QString
        const QString result = QString::fromUtf8(converted_text);
        // 3. 释放 FFI 函数分配的字符串内存
        opencc_free_string(converted_text);
        return result;
    } else {
        qWarning() << "OpenCC conversion failed, returning original text.";
        return input_text; // 转换失败则返回原文
    }
}

bool tool::utils::OpenCCConverter::isValid() const {
    return _is_valid;
}
