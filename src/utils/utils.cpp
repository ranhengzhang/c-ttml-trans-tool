//
// Created by LEGION on 2026/2/12.
//

#include <QDebug>
#include <QEventLoop>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpression>
#include <QString>

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

QStringList tool::utils::getQids(const QString &vid) {
    QStringList result;

    //region 构建请求 JSON
    QJsonObject comm_obj;
    comm_obj["ct"] = "26";
    comm_obj["cv"] = "2010101";
    comm_obj["v"] = "2010101";

    QJsonObject param_obj;
    param_obj["types"] = QJsonArray{1};
    param_obj["ctx"] = 0;
    if (vid.startsWith("00")) {
        param_obj["mids"] = QJsonArray{vid};
    } else {
        param_obj["ids"] = QJsonArray{vid.toLongLong()};
    }

    QJsonObject req_obj;
    req_obj["module"] = "music.trackInfo.UniformRuleCtrl";
    req_obj["method"] = "CgiGetTrackInfo";
    req_obj["param"] = param_obj;

    QJsonObject root_obj;
    root_obj["comm"] = comm_obj;
    root_obj["req"] = req_obj;

    const QJsonDocument json_doc(root_obj);
    const QByteArray post_data = json_doc.toJson();
    //endregion

    // 使用封装的 httpPost 发送请求
    auto response = httpPost(
        QUrl("https://u.y.qq.com/cgi-bin/musicu.fcg"),
        post_data,
        "application/json",
        10000
    );

    if (!response.success || response.content.isEmpty()) {
        return result;
    }

    // 处理响应：将 `: undefined` 替换为 `: null`
    const QRegularExpression re(R"((:\s*)undefined\b)");
    const QString content = response.content.replace(re, "\\1null");

    // 解析 JSON 响应
    const auto json = QJsonDocument::fromJson(content.toUtf8());
    if (!json.isObject()) {
        return result;
    }

    const auto json_obj = json.object();
    const auto req_val = json_obj["req"];
    if (!req_val.isObject()) {
        return result;
    }

    const auto res_obj = req_val.toObject();
    const auto data_val = res_obj["data"];
    if (!data_val.isObject()) {
        return result;
    }

    const auto data_obj = data_val.toObject();
    const auto tracks_val = data_obj["tracks"];
    if (!tracks_val.isArray() || tracks_val.toArray().isEmpty()) {
        return result;
    }

    const auto song = tracks_val.toArray()[0].toObject();

    const auto mid = song["mid"].toString();
    if (!mid.isEmpty() and mid != vid) {
        result.push_back(mid);
    }
    const auto id = QString::number(song["id"].toInteger(-1));
    if (id != "-1" and id != vid) {
        result.push_back(id);
    }

    return result;
}

tool::utils::NetworkResponse tool::utils::httpGet(const QUrl &url, const int timeout_ms) {
    NetworkResponse response;
    response.success = false;
    response.statusCode = 0;

    if (!url.isValid()) {
        response.errorString = "Invalid URL";
        return response;
    }

    QNetworkAccessManager manager;
    const QNetworkRequest request(url);
    QNetworkReply *reply = manager.get(request);

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, [&]() {
        loop.quit();
        reply->abort();
    });

    timer.start(timeout_ms);
    loop.exec();

    const bool isTimeout = !timer.isActive();
    if (isTimeout) {
        response.errorString = "Request timeout";
        reply->deleteLater();
        return response;
    }

    if (reply->error() != QNetworkReply::NoError) {
        response.errorString = reply->errorString();
        reply->deleteLater();
        return response;
    }

    response.success = true;
    response.statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    response.content = QString::fromUtf8(reply->readAll());
    reply->deleteLater();

    return response;
}

tool::utils::NetworkResponse tool::utils::httpPost(const QUrl &url, const QByteArray &data,
                                                   const QString &contentType, const int timeoutMs) {
    NetworkResponse response;
    response.success = false;
    response.statusCode = 0;

    if (!url.isValid()) {
        response.errorString = "Invalid URL";
        return response;
    }

    QNetworkAccessManager manager;
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, contentType);

    QNetworkReply *reply = manager.post(request, data);

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, [&]() {
        loop.quit();
        reply->abort();
    });

    timer.start(timeoutMs);
    loop.exec();

    const bool isTimeout = !timer.isActive();
    if (isTimeout) {
        response.errorString = "Request timeout";
        reply->deleteLater();
        return response;
    }

    if (reply->error() != QNetworkReply::NoError) {
        response.errorString = reply->errorString();
        reply->deleteLater();
        return response;
    }

    response.success = true;
    response.statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    response.content = QString::fromUtf8(reply->readAll());
    reply->deleteLater();

    return response;
}
