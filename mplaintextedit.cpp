#include "mplaintextedit.h"

#include <QFile>
#include <QMimeData>
#include <QMessageBox>

MPlainTextEdit::MPlainTextEdit(QWidget *parent)
    : QPlainTextEdit{parent}
{
    this->setAcceptDrops(true);

    connect(this, &QPlainTextEdit::textChanged, this, &MPlainTextEdit::onTextChanged);
}

void MPlainTextEdit::setTextProgrammatically(const QString &text) {
    this->_isProgrammaticChange = false;
    this->setPlainText(text);
}

void MPlainTextEdit::dragEnterEvent(QDragEnterEvent *event) {
    // 检查拖入的文件是否为 .ttml 文件
    // ReSharper disable once CppTooWideScopeInitStatement
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        // ReSharper disable once CppTooWideScopeInitStatement
        QList<QUrl> urlList = mimeData->urls();
        if (!urlList.isEmpty()) {
            // ReSharper disable once CppTooWideScopeInitStatement
            const QString filePath = urlList.first().toLocalFile();
            if (filePath.endsWith(".ttml", Qt::CaseInsensitive)) {
                event->acceptProposedAction();  // 接受拖入操作
            } else {
                event->ignore();  // 忽略非 .ttml 文件
            }
        }
    } else {
        event->ignore();  // 如果没有文件，不接受
    }
}

void MPlainTextEdit::dropEvent(QDropEvent *event) {
    // ReSharper disable once CppTooWideScopeInitStatement
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        // ReSharper disable once CppTooWideScopeInitStatement
        QList<QUrl> urlList = mimeData->urls();
        if (!urlList.isEmpty()) {
            // ReSharper disable once CppTooWideScopeInitStatement
            const QString filePath = urlList.first().toLocalFile();
            if (filePath.endsWith(".ttml", Qt::CaseInsensitive)) {
                // 读取文件内容并显示在文本框中
                QFile file(filePath);
                if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    QTextStream in(&file);
                    this->setTextProgrammatically(in.readAll());  // 读取文件内容并显示
                } else {
                    // 文件读取失败，弹出提示框
                    QMessageBox::warning(this, "文件错误", QString("无法打开文件: %1").arg(filePath));
                }
            }
        }
    }
}

void MPlainTextEdit::onTextChanged() {
    if (!this->_isProgrammaticChange) {
        emit textEdited();
        this->_isProgrammaticChange = true;
    }
}
