//
// Created by LEGION on 2026/2/12.
//

#ifndef TTML_TOOL_UTILS_H
#define TTML_TOOL_UTILS_H

#include "ferrous_opencc/opencc.h"

namespace tool::utils {
    // 一个 C++ 包装类，用于管理 OpenCC 实例的生命周期
    // ReSharper disable once CppInconsistentNaming
    class OpenCCConverter {
    public:
        // 构造函数，传入所需的转换配置
        explicit OpenCCConverter(BuiltinConfig config);

        // 析构函数，自动释放 OpenCC 实例
        ~OpenCCConverter();

        // 执行文本转换
        [[nodiscard]] QString convert(const QString &input_text) const;

        // 检查转换器实例是否成功创建
        [[nodiscard]] bool isValid() const;

        // 禁止拷贝构造和赋值，避免对同一个 C-style handle 的重复管理
        OpenCCConverter(const OpenCCConverter &) = delete;
        OpenCCConverter &operator=(const OpenCCConverter &) = delete;

    private:
        OpenCCHandle *_handle{}; // OpenCC 的不透明句柄
        bool _is_valid;         // 标记句柄是否有效
    };
};


#endif //TTML_TOOL_UTILS_H
