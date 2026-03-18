//
// Created by LEGION on 2026/2/12.
//

#ifndef TTML_TOOL_UTILS_H
#define TTML_TOOL_UTILS_H

#include <QNetworkReply>
#include <QStringList>

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

    /**
     * 根据 QQ 音乐 ID 查询歌曲信息（mid, vid, title, subtitle）
     * @param vid QQ 音乐 ID（可以是数字 ID 或 mid）
     * @return QidResult 包含查询结果的结构体，查询失败时各字段为空
     */
    QStringList getQids(const QString &vid);

    /**
     * 同步网络请求结果结构体
     */
    struct NetworkResponse {
        bool success;           // 请求是否成功
        int statusCode;         // HTTP 状态码
        QString content;        // 响应内容
        QString errorString;    // 错误信息（失败时有效）
    };

    /**
     * 执行同步 HTTP GET 请求
     * @param url 请求的 URL
     * @param timeoutMs 超时时间（毫秒），默认 10 秒
     * @return NetworkResponse 包含请求结果的结构体
     */
    NetworkResponse httpGet(const QUrl &url, int timeoutMs = 10000);

    /**
     * 执行同步 HTTP POST 请求
     * @param url 请求的 URL
     * @param data POST 数据
     * @param contentType Content-Type 头部，默认 application/x-www-form-urlencoded
     * @param timeoutMs 超时时间（毫秒），默认 10 秒
     * @return NetworkResponse 包含请求结果的结构体
     */
    NetworkResponse httpPost(const QUrl &url, const QByteArray &data,
                             const QString &contentType = "application/x-www-form-urlencoded",
                             int timeoutMs = 10000);
};


#endif //TTML_TOOL_UTILS_H
