#include <memory>

#include <QMessageBox>
#include <QThread>
#include <QFileDialog>
#include <QClipboard>
#include <QInputDialog>

#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "lyric.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
      , ui(new Ui::MainWindow) {
    ui->setupUi(this);

    ui->TTMLTextEdit->setWordWrapMode(QTextOption::WrapAnywhere);

    // ReSharper disable once CppTooWideScopeInitStatement
    const auto mono_font = QFontDatabase::addApplicationFont(R"(://asset/Sarasa.ttf)");
    if (mono_font != -1) {
        // ReSharper disable once CppTooWideScopeInitStatement
        const QStringList string_list(QFontDatabase::applicationFontFamilies(mono_font));

        if (string_list.count()) {
            QFont m_font(string_list.at(0));
            m_font.setPointSize(14);
            ui->TTMLTextEdit->setFont(m_font);
        }
    }

    opencc_create("s2t.json", &ot_s2t);
    opencc_create("t2s.json", &ot_t2s);
}

MainWindow::~MainWindow() {
    delete ui;

    opencc_destroy(ot_t2s);
    opencc_destroy(ot_s2t);
}

void MainWindow::on_offsetButton_clicked() {
    ui->offsetButton->setEnabled(false); // 禁用按钮防止重复点击

    QString text = ui->TTMLTextEdit->toPlainText();
    const int offset = ui->offsetCount->value();

    ui->statusbar->showMessage(R"(开始偏移时间)");
    this->setEnabled(false);

    // ReSharper disable once CppTooWideScopeInitStatement
    int8_t sign = 1;

    auto pos = text.indexOf(R"(dur=")");
    if (pos == -1) {
        ui->statusbar->showMessage(R"(无法查找 dur)");
        this->setEnabled(true);
        ui->offsetButton->setEnabled(true);
        return;
    }

    while (pos != -1) {
        const auto valueStart = pos + (sign & 1 ? 5 : 7);
        // ReSharper disable once CppTooWideScopeInitStatement
        const auto valueEnd = text.indexOf('"', valueStart);

        if (valueEnd == -1) {
            ui->statusbar->showMessage(QString(R"(无法查找时间于：%1)").arg(valueStart));
            this->setEnabled(true);
            ui->offsetButton->setEnabled(true);
            return;
        }

        auto time_str = text.mid(valueStart, valueEnd - valueStart);
        ui->statusbar->showMessage(QString(R"(处理时间戳：%1)").arg(time_str));
        // ReSharper disable once CppTooWideScopeInitStatement
        // 时间处理逻辑
        bool ok;
        auto time = LyricTime::parse(time_str, &ok);
        if (!ok) {
            ui->statusbar->showMessage(QString(R"(时间戳格式错误：%1 [%2])").arg(time_str).arg(valueStart));
            this->setEnabled(true);
            ui->offsetButton->setEnabled(true);
            return;
        }

        time.offset(offset);
        text.replace(valueStart,
                     valueEnd - valueStart,
                     time.toString(false, false, true));

        // ReSharper disable once CppCompileTimeConstantCanBeReplacedWithBooleanConstant
        if (sign & 1) {
            // ReSharper disable once CppDFAUnreachableCode
            pos = text.indexOf(R"(begin=")", valueEnd);
            // ReSharper disable once CppRedundantElseKeywordInsideCompoundStatement
        } else {
            pos = text.indexOf(R"(end=")", valueEnd);
        }

        sign ^= 1;
    }

    ui->TTMLTextEdit->setPlainText(text);
    this->setEnabled(true);
    ui->offsetButton->setEnabled(true);
    ui->statusbar->showMessage(R"(时间偏移完成)");
}

void MainWindow::on_fromFile_triggered() {
    const auto file_path = QFileDialog::getOpenFileName(this, R"(选择文件)", "", R"(Timed Text Markup Language Files (*.ttml))");

    if (file_path.isEmpty()) {
        QMessageBox::warning(this, R"(警告)", R"(未选择文件)");
        return;
    }

    const auto file = new QFile(file_path);

    if (!file->open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, R"(错误)", QString(R"(无法打开文件：%1)").arg(file_path));
        return;
    }

    const auto text = file->readAll();

    file->close();
    delete file;

    ui->TTMLTextEdit->clear();
    ui->TTMLTextEdit->setPlainText(text);
}


void MainWindow::on_fromClipboard_triggered() {
    const auto text = QApplication::clipboard()->text();

    if (text.isEmpty()) {
        QMessageBox::critical(this, R"(错误)", R"(剪贴板为空)");
        return;
    }

    ui->TTMLTextEdit->clear();
    ui->TTMLTextEdit->setPlainText(text);
}

void MainWindow::on_toTTML_triggered() {
    // ReSharper disable once CppTooWideScopeInitStatement
    const auto ok = this->parse();

    if (!ok) return;

    const auto file_path = QFileDialog::getSaveFileName(this, R"(选择文件)", this->_lyric->getTitle(R"(ttml)"),
                                                        R"(Timed Text Markup Language File (*.ttml))");

    if (file_path.isEmpty()) {
        QMessageBox::warning(this, R"(警告)", R"(未选择文件)");
        return;
    }

    this->setEnabled(false);
    ui->statusbar->showMessage(R"(TTML 生成中)");

    const auto text = this->_lyric->toTTML();
    const auto file = new QFile(file_path);
    if (!file->open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, R"(错误)", QString(R"(无法打开文件：%1)").arg(file_path));
        this->setEnabled(true);
        return;
    }

    file->write(text.toUtf8());
    file->close();
    delete file;

    this->setEnabled(true);
    ui->statusbar->showMessage(R"(TTML 生成完成)");
}


void MainWindow::on_toASS_triggered() {
    // ReSharper disable once CppTooWideScopeInitStatement
    const auto ok = this->parse();

    if (!ok) return;

    const auto file_path = QFileDialog::getSaveFileName(this, R"(选择文件)", this->_lyric->getTitle(R"(ass)"),
                                                        R"(Advanced SubStation Alpha File (*.ass))");

    if (file_path.isEmpty()) {
        QMessageBox::warning(this, R"(警告)", R"(未选择文件)");
        return;
    }

    this->setEnabled(false);
    ui->statusbar->showMessage(R"(ASS 生成中)");

    const auto text = this->_lyric->toASS();
    const auto file = new QFile(file_path);
    if (!file->open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, R"(错误)", QString(R"(无法打开文件：%1)").arg(file_path));
        this->setEnabled(true);
        return;
    }

    file->write(text.toUtf8());
    file->close();
    delete file;

    this->setEnabled(true);
    ui->statusbar->showMessage(R"(ASS 生成完成)");
}

bool MainWindow::parse() {
    if (this->_lyric) {
        return true;
    }

    const auto text = ui->TTMLTextEdit->toPlainText();
    QDomDocument doc;

    // ReSharper disable once CppTooWideScopeInitStatement
    const auto res = doc.setContent(text, QDomDocument::ParseOption::PreserveSpacingOnlyNodes);

    if (!res) {
        QMessageBox::critical(this, R"(错误)",
                              QString("无法解析TTML于 (%1,%2)\n%3")
                              .arg(res.errorLine)
                              .arg(res.errorLine)
                              .arg(res.errorMessage));
        ui->statusbar->showMessage(R"(TTML 解析失败)");
        return false;
    }

    bool ok;
    auto lrc = Lyric::parse(doc.documentElement(), &ok);

    if (!ok) {
        QMessageBox::critical(this, R"(错误)", R"(无法解析 TTML)");
        ui->statusbar->showMessage(R"(TTML 解析失败)");
        return false;
    }

    this->_lyric = std::make_unique<Lyric>(std::move(lrc));
    return true;
}

void MainWindow::on_TTMLTextEdit_textChanged() {
    this->_lyric.reset();
}

void MainWindow::on_toLRC_triggered() {
    // ReSharper disable once CppTooWideScopeInitStatement
    auto ok = this->parse();

    if (!ok) return;

    const auto file_path = QFileDialog::getSaveFileName(this, R"(选择文件)", this->_lyric->getTitle(R"(lrc)"),
                                                        R"(Lyric File (*.lrc))");

    if (file_path.isEmpty()) {
        QMessageBox::warning(this, R"(警告)", R"(未选择文件)");
        return;
    }

    QStringList options{};

    options.append(R"(无)");
    if (this->_lyric->haveRoman()) {
        options.append(R"(x-roman - 罗马音)");
    }
    if (!this->_lyric->getLangs().isEmpty()) {
        for (const auto &lang: this->_lyric->getLangs()) {
            options.append(QString(R"(x-trans - 翻译 - lang:%1)").arg(lang));
        }
    }

    QString extra{};
    if (options.size() > 1)
        extra = QInputDialog::getItem(this, R"(选择额外信息)", R"(选择额外信息)", options, 0, false, &ok);

    if (!ok) {
        QMessageBox::warning(this, R"(警告)", R"(取消选择额外信息)");
        return;
    }

    this->setEnabled(false);
    ui->statusbar->showMessage(R"(LRC 生成中)");

    const auto text = this->_lyric->toLRC(extra);
    const auto file = new QFile(file_path);
    if (!file->open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, R"(错误)", QString(R"(无法打开文件：%1)").arg(file_path));
        this->setEnabled(true);
        return;
    }

    file->write(text.toUtf8());
    file->close();
    delete file;

    this->setEnabled(true);
    ui->statusbar->showMessage(R"(LRC 生成完成)");
}

void MainWindow::on_toLYS_triggered() {
    // ReSharper disable once CppTooWideScopeInitStatement
    auto ok = this->parse();

    if (!ok) return;

select_lys:
    const auto orig_file_path = QFileDialog::getSaveFileName(this, R"(选择文件)", this->_lyric->getTitle(R"(lys)"),
                                                             R"(Lyricify Syllable File (*.lys))");

    if (orig_file_path.isEmpty()) {
        QMessageBox::warning(this, R"(警告)", R"(未选择文件)");
        return;
    }

    const auto file_info = QFileInfo(orig_file_path);
    const auto dir = file_info.absoluteDir();
    const auto basename = file_info.baseName();
    // ReSharper disable once CppTooWideScopeInitStatement
    const auto ts_file_path = dir.absoluteFilePath(QString(R"(%1_trans.lrc)").arg(basename));

    if (!this->_lyric->getLangs().isEmpty() && QFileInfo::exists(ts_file_path)) {
        const QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            R"(确认覆盖文件)",
            QString(R"(文件 %1_trans.lrc 已存在，确定要覆盖吗？)").arg(basename),
            QMessageBox::Yes | QMessageBox::Cancel
        );

        if (reply != QMessageBox::Yes)
            goto select_lys;
    }

    QString lang{};

    if (this->_lyric->getLangs().size() > 1) {
        lang = QInputDialog::getItem(this, R"(选择语言)", R"(翻译语言)", this->_lyric->getLangs(), 0, false, &ok);

        if (!ok) {
            QMessageBox::warning(this, R"(警告)", R"(取消选择语言)");
            return;
        }
    } else if (this->_lyric->getLangs().size() == 1)
        lang = this->_lyric->getLangs().first();

    this->setEnabled(false);
    ui->statusbar->showMessage(R"(LYS 生成中)");

    const auto [orig, ts] = this->_lyric->toLYS(lang);
    const auto orig_file = new QFile(orig_file_path);
    if (!orig_file->open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, R"(错误)", QString(R"(无法打开文件：%1)").arg(orig_file_path));
        this->setEnabled(true);
        return;
    }

    orig_file->write(orig.toUtf8());
    orig_file->close();
    delete orig_file;

    if (!this->_lyric->getLangs().isEmpty()) {
        const auto ts_file = new QFile(ts_file_path);
        if (!ts_file->open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::critical(this, R"(错误)", QString(R"(无法打开文件：%1)").arg(ts_file_path));
            this->setEnabled(true);
            return;
        }

        ts_file->write(ts.toUtf8());
        ts_file->close();
        delete ts_file;
    }

    this->setEnabled(true);
    ui->statusbar->showMessage(R"(LYS 生成完成)");
}

void MainWindow::on_toSPL_triggered() {
    // ReSharper disable once CppTooWideScopeInitStatement
    const auto ok = this->parse();

    if (!ok) return;

    const auto file_path = QFileDialog::getSaveFileName(this, R"(选择文件)", this->_lyric->getTitle(R"(lrc)"),
                                                        R"(Salt Player Lyrics File (*.lrc))");

    if (file_path.isEmpty()) {
        QMessageBox::warning(this, R"(警告)", R"(未选择文件)");
        return;
    }

    this->setEnabled(false);
    ui->statusbar->showMessage(R"(SPL 生成中)");

    const auto text = this->_lyric->toSPL();
    const auto file = new QFile(file_path);
    if (!file->open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, R"(错误)", QString(R"(无法打开文件：%1)").arg(file_path));
        this->setEnabled(true);
        return;
    }

    file->write(text.toUtf8());
    file->close();
    delete file;

    this->setEnabled(true);
    ui->statusbar->showMessage(R"(SPL 生成完成)");
}

void MainWindow::on_toQRC_triggered() {
    // ReSharper disable once CppTooWideScopeInitStatement
    auto ok = this->parse();

    if (!ok) return;

select_qrc:
    const auto orig_file_path = QFileDialog::getSaveFileName(this, R"(选择文件)", this->_lyric->getTitle(R"(qrc)"),
                                                             R"(QQ Music Lyrics File (*.qrc))");

    if (orig_file_path.isEmpty()) {
        QMessageBox::warning(this, R"(警告)", R"(未选择文件)");
        return;
    }

    const auto file_info = QFileInfo(orig_file_path);
    const auto dir = file_info.absoluteDir();
    const auto basename = file_info.baseName();
    // ReSharper disable once CppTooWideScopeInitStatement
    const auto ts_file_path = dir.absoluteFilePath(QString(R"(%1.translrc)").arg(basename));

    if (!this->_lyric->getLangs().isEmpty() && QFileInfo::exists(ts_file_path)) {
        const QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            R"(确认覆盖文件)",
            QString(R"(文件 %1_trans.lrc 已存在，确定要覆盖吗？)").arg(basename),
            QMessageBox::Yes | QMessageBox::Cancel
        );

        if (reply != QMessageBox::Yes)
            goto select_qrc;
    }

    // ReSharper disable once CppTooWideScopeInitStatement
    const auto roman_file_path = dir.absoluteFilePath(QString(R"(%1.romaqrc)").arg(basename));

    if (this->_lyric->haveRoman() && QFileInfo::exists(roman_file_path)) {
        const QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            R"(确认覆盖文件)",
            QString(R"(文件 %1.romaqrc 已存在，确定要覆盖吗？)").arg(basename),
            QMessageBox::Yes | QMessageBox::Cancel
        );

        if (reply != QMessageBox::Yes)
            goto select_qrc;
    }

    QString lang{};

    if (this->_lyric->getLangs().size() > 1) {
        lang = QInputDialog::getItem(this, R"(选择语言)", R"(翻译语言)", this->_lyric->getLangs(), 0, false, &ok);

        if (!ok) {
            QMessageBox::warning(this, R"(警告)", R"(取消选择语言)");
            return;
        }
    } else if (this->_lyric->getLangs().size() == 1)
        lang = this->_lyric->getLangs().first();

    this->setEnabled(false);
    ui->statusbar->showMessage(R"(QRC 生成中)");

    const auto qrc = this->_lyric->toQRC(lang);
    const auto orig_file = new QFile(orig_file_path);
    if (!orig_file->open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, R"(错误)", QString(R"(无法打开文件：%1)").arg(orig_file_path));
        this->setEnabled(true);
        return;
    }

    orig_file->write(qrc[R"(orig)"].toUtf8());
    orig_file->close();
    delete orig_file;

    if (!this->_lyric->getLangs().isEmpty()) {
        const auto ts_file = new QFile(ts_file_path);
        if (!ts_file->open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::critical(this, R"(错误)", QString(R"(无法打开文件：%1)").arg(ts_file_path));
            this->setEnabled(true);
            return;
        }

        ts_file->write(qrc[R"(ts)"].toUtf8());
        ts_file->close();
        delete ts_file;
    }

    if (this->_lyric->haveRoman()) {
        const auto roman_file = new QFile(roman_file_path);
        if (!roman_file->open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::critical(this, R"(错误)", QString(R"(无法打开文件：%1)").arg(roman_file_path));
            this->setEnabled(true);
            return;
        }

        roman_file->write(qrc[R"(roman)"].toUtf8());
        roman_file->close();
        delete roman_file;
    }

    this->setEnabled(true);
    ui->statusbar->showMessage(R"(QRC 生成完成)");
}

void MainWindow::on_toYRC_triggered() {
    // ReSharper disable once CppTooWideScopeInitStatement
    auto ok = this->parse();

    if (!ok) return;

select_lys:
    const auto orig_file_path = QFileDialog::getSaveFileName(this, R"(选择文件)", this->_lyric->getTitle("yrc"),
                                                             R"(Netease Cloud Music Lyrics File (*.yrc))");

    if (orig_file_path.isEmpty()) {
        QMessageBox::warning(this, R"(警告)", R"(未选择文件)");
        return;
    }

    const auto file_info = QFileInfo(orig_file_path);
    const auto dir = file_info.absoluteDir();
    const auto basename = file_info.baseName();
    // ReSharper disable once CppTooWideScopeInitStatement
    const auto ts_file_path = dir.absoluteFilePath(QString(R"(%1.lrc)").arg(basename));

    if (!this->_lyric->getLangs().isEmpty() && QFileInfo::exists(ts_file_path)) {
        const QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            R"(确认覆盖文件)",
            QString(R"(文件 %1.lrc 已存在，确定要覆盖吗？)").arg(basename),
            QMessageBox::Yes | QMessageBox::Cancel
        );

        if (reply != QMessageBox::Yes)
            goto select_lys;
    }

    QString lang{};

    if (this->_lyric->getLangs().size() > 1) {
        lang = QInputDialog::getItem(this, R"(选择语言)", R"(翻译语言)", this->_lyric->getLangs(), 0, false, &ok);

        if (!ok) {
            QMessageBox::warning(this, R"(警告)", R"(取消选择语言)");
            return;
        }
    } else if (this->_lyric->getLangs().size() == 1)
        lang = this->_lyric->getLangs().first();

    this->setEnabled(false);
    ui->statusbar->showMessage(R"(YRC 生成中)");

    const auto [orig, ts] = this->_lyric->toYRC(lang);
    const auto orig_file = new QFile(orig_file_path);
    if (!orig_file->open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, R"(错误)", QString(R"(无法打开文件：%1)").arg(orig_file_path));
        this->setEnabled(true);
        return;
    }

    orig_file->write(orig.toUtf8());
    orig_file->close();
    delete orig_file;

    if (!this->_lyric->getLangs().isEmpty()) {
        const auto ts_file = new QFile(ts_file_path);
        if (!ts_file->open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::critical(this, R"(错误)", QString(R"(无法打开文件：%1)").arg(ts_file_path));
            this->setEnabled(true);
            return;
        }

        ts_file->write(ts.toUtf8());
        ts_file->close();
        delete ts_file;
    }

    this->setEnabled(true);
    ui->statusbar->showMessage(R"(YRC 生成完成)");
}


void MainWindow::on_toTXT_triggered() {
    // ReSharper disable once CppTooWideScopeInitStatement
    const auto ok = this->parse();

    if (!ok) return;

    const auto file_path = QFileDialog::getSaveFileName(this, R"(选择文件)", this->_lyric->getTitle(R"(txt)"),
                                                        R"(Text File (*.txt))");

    if (file_path.isEmpty()) {
        QMessageBox::warning(this, R"(警告)", R"(未选择文件)");
        return;
    }

    this->setEnabled(false);
    ui->statusbar->showMessage(R"(TXT 生成中)");

    const auto text = this->_lyric->toTXT();
    const auto file = new QFile(file_path);
    if (!file->open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, R"(错误)", QString(R"(无法打开文件：%1)").arg(file_path));
        this->setEnabled(true);
        return;
    }

    file->write(text.toUtf8());
    file->close();
    delete file;

    this->setEnabled(true);
    ui->statusbar->showMessage(R"(TXT 生成完成)");
}

void MainWindow::on_fromLYS_triggered()
{

}


void MainWindow::on_fromQRC_triggered()
{

}


void MainWindow::on_fromYRC_triggered()
{

}


void MainWindow::on_copyTTML_triggered()
{
    // ReSharper disable once CppTooWideScopeInitStatement
    const auto ok = this->parse();

    if (!ok) return;

    const auto text = this->_lyric->toTTML();
    QApplication::clipboard()->setText(text);
}

void MainWindow::on_copyASS_triggered()
{
    // ReSharper disable once CppTooWideScopeInitStatement
    const auto ok = this->parse();

    if (!ok) return;

    const auto text = this->_lyric->toASS();
    QApplication::clipboard()->setText(text);
    ui->statusbar->showMessage("导出成功");
}

void MainWindow::on_copyLRC_triggered()
{
    // ReSharper disable once CppTooWideScopeInitStatement
    const auto ok = this->parse();

    if (!ok) return;

    const auto text = this->_lyric->toLRC("");
    QApplication::clipboard()->setText(text);
    ui->statusbar->showMessage("导出成功");
}

void MainWindow::on_copySPL_triggered()
{
    // ReSharper disable once CppTooWideScopeInitStatement
    const auto ok = this->parse();

    if (!ok) return;

    const auto text = this->_lyric->toSPL();
    QApplication::clipboard()->setText(text);
    ui->statusbar->showMessage("导出成功");
}


void MainWindow::on_copyLYS_triggered()
{
    // ReSharper disable once CppTooWideScopeInitStatement
    const auto ok = this->parse();

    if (!ok) return;

    const auto text = this->_lyric->toLYS("");
    QApplication::clipboard()->setText(text.first);
    ui->statusbar->showMessage("导出成功");
}


void MainWindow::on_copyQRC_triggered()
{
    // ReSharper disable once CppTooWideScopeInitStatement
    const auto ok = this->parse();

    if (!ok) return;

    const auto text = this->_lyric->toQRC("");
    QApplication::clipboard()->setText(text[R"(orig)"]);
    ui->statusbar->showMessage("导出成功");
}


void MainWindow::on_copyYRC_triggered()
{
    // ReSharper disable once CppTooWideScopeInitStatement
    const auto ok = this->parse();

    if (!ok) return;

    const auto text = this->_lyric->toYRC("");
    QApplication::clipboard()->setText(text.first);
    ui->statusbar->showMessage("导出成功");
}


void MainWindow::on_copyTXT_triggered()
{
    // ReSharper disable once CppTooWideScopeInitStatement
    const auto ok = this->parse();

    if (!ok) return;

    const auto text = this->_lyric->toTXT();
    QApplication::clipboard()->setText(text);
    ui->statusbar->showMessage("导出成功");
}

QString MainWindow::s2t(QString val) {
    auto str = val.toStdString();
    auto buffer = opencc_convert(ot_s2t, str.c_str());
    return QString::fromUtf8(buffer);
}

QString MainWindow::t2s(QString val) {
    auto str = val.toStdString();
    auto buffer = opencc_convert(ot_t2s, str.c_str());
    return QString::fromUtf8(buffer);
}

void MainWindow::node_t2s(QDomNode &node) {
    // 递归终止条件
    if (node.isNull()) return;

    // 处理当前节点
    if (node.isElement()) {
        const QDomElement el = node.toElement();

        // 跳过metadata标签
        if (el.tagName() == "metadata") {
            return;
        }

        // 跳过带有特定属性的span
        if (el.tagName() == "span") {
            if (el.hasAttribute("ttm:role") &&
                el.attribute("ttm:role") == "x-translation") {
                return;
                }
        }
    }

    // 处理文本节点
    if (node.isText()) {
        const QString content = node.nodeValue();
        if (!content.trimmed().isEmpty()) {
            node.setNodeValue(t2s(content));
        }
    }

    // 递归处理子节点
    QDomNode child = node.firstChild();
    while (!child.isNull()) {
        const QDomNode next = child.nextSibling();  // 先获取下一个兄弟节点
        node_t2s(child);                        // 递归处理当前子节点
        child = next;                               // 移动到下一个兄弟节点
    }
}

void MainWindow::node_s2t(QDomNode &node) {
    // 递归终止条件
    if (node.isNull()) return;

    // 处理当前节点
    if (node.isElement()) {
        const QDomElement el = node.toElement();

        // 跳过metadata标签
        if (el.tagName() == "metadata") {
            return;
        }

        // 跳过带有特定属性的span
        if (el.tagName() == "span") {
            if (el.hasAttribute("ttm:role") &&
                el.attribute("ttm:role") == "x-translation") {
                return;
                }
        }
    }

    // 处理文本节点
    if (node.isText()) {
        const QString content = node.nodeValue();
        if (!content.trimmed().isEmpty()) {
            node.setNodeValue(s2t(content));
        }
    }

    // 递归处理子节点
    QDomNode child = node.firstChild();
    while (!child.isNull()) {
        const QDomNode next = child.nextSibling();  // 先获取下一个兄弟节点
        node_s2t(child);                        // 递归处理当前子节点
        child = next;                               // 移动到下一个兄弟节点
    }
}

void MainWindow::on_actions2t_triggered()
{
    const auto text = ui->TTMLTextEdit->toPlainText();
    QDomDocument doc;

    // ReSharper disable once CppTooWideScopeInitStatement
    const auto res = doc.setContent(text, QDomDocument::ParseOption::PreserveSpacingOnlyNodes);

    if (!res) {
        QMessageBox::critical(this, R"(错误)",
                              QString("无法解析TTML于 (%1,%2)\n%3")
                              .arg(res.errorLine)
                              .arg(res.errorLine)
                              .arg(res.errorMessage));
        ui->statusbar->showMessage(R"(TTML 解析失败)");
        return;
    }

    node_s2t(doc);
    ui->TTMLTextEdit->setPlainText(doc.toString(-1));
}

void MainWindow::on_actiont2s_triggered()
{
    const auto text = ui->TTMLTextEdit->toPlainText();
    QDomDocument doc;

    // ReSharper disable once CppTooWideScopeInitStatement
    const auto res = doc.setContent(text, QDomDocument::ParseOption::PreserveSpacingOnlyNodes);

    if (!res) {
        QMessageBox::critical(this, R"(错误)",
                              QString("无法解析TTML于 (%1,%2)\n%3")
                              .arg(res.errorLine)
                              .arg(res.errorLine)
                              .arg(res.errorMessage));
        ui->statusbar->showMessage(R"(TTML 解析失败)");
        return;
    }

    node_t2s(doc);
    ui->TTMLTextEdit->setPlainText(doc.toString(-1));
}

void MainWindow::on_actionPreset_triggered()
{
    // ReSharper disable once CppTooWideScopeInitStatement
    auto ok = this->parse();

    if (!ok) return;

    auto metas = this->_lyric->getPresetMeta();

    if (metas.isEmpty())
        return;

    QStringList buffer{};

    for (const auto &[key, alt] : Lyric::presetMetas) {
        if (metas.contains(key)) {
            buffer.append("### " + alt);

            for (const auto &meta : metas[key])
                buffer.append("- " + ("`" + meta + "`"));
        }
    }

    QApplication::clipboard()->setText(buffer.join('\n'));
    ui->statusbar->showMessage("复制成功");
}


void MainWindow::on_actionExtra_triggered()
{
    // ReSharper disable once CppTooWideScopeInitStatement
    auto ok = this->parse();

    if (!ok) return;

    auto metas = this->_lyric->getExtraMeta();
    QStringList buffer{"扩展元数据：", "| *key* | *value* |", "| -: | :- |"};

    for (const auto &[key, value] : metas) {
        buffer.append(QString("| **%1** | `%2` |").arg(key).arg(value));
    }

    QApplication::clipboard()->setText(buffer.join('\n'));
    ui->statusbar->showMessage("复制成功");
}
