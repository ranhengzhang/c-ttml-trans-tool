#include <memory>

#include <QMessageBox>
#include <QThread>
#include <QFileDialog>
#include <QClipboard>
#include <QInputDialog>
#include <QStack>
#include <QNetworkReply>
#include <QTimer>

#include "MainWindow.h"

#include <random>

#include "./ui_mainwindow.h"
#include "LangSelectDialog.h"
#include "LyricObject.h"
#include "TimeFormatDialog.h"

QList<std::pair<QString, QString>> preset_metas{
            {"musicName", "音乐名称"},
            {"artists", "音乐作者"},
            {"album", "音乐专辑名称"},
            {"ncmMusicId", "歌曲关联网易云音乐 ID"},
            {"qqMusicId", "歌曲关联 QQ 音乐 ID"},
            {"spotifyId", "歌曲关联 Spotify 音乐 ID"},
            {"appleMusicId", "歌曲关联 Apple Music 音乐 ID"},
            {"isrc", "歌曲关联 ISRC"}
};

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

    ui->offsetCount->setMinimum(INT32_MIN);
    ui->offsetCount->setMaximum(INT32_MAX);
}

MainWindow::~MainWindow() {
    delete ui;

    opencc_destroy(ot_t2s);
    opencc_destroy(ot_s2t);
}

std::tuple<QString, bool> offsetWithKey(const QString &key, QString &text, const int64_t offset) {
    auto begin_pos = text.indexOf(QString(R"(%1=")").arg(key));
    QList<std::tuple<int64_t, int64_t, QString>> time_list;

    while (begin_pos != -1) {
        auto end_pos = text.indexOf('\"', begin_pos + key.length() + 2);
        if (end_pos > begin_pos) {
            auto time_str = text.mid(begin_pos + key.length() + 2, end_pos - begin_pos - key.length() - 2);
            auto [time, success] = LyricTime::parse(time_str);
            if (success) {
                time.offset(offset);
                time_list.push_back(std::make_tuple(begin_pos + key.length() + 2, end_pos - begin_pos - key.length() - 2, time.toString(false, false, true)));
                begin_pos = text.indexOf(QString(R"(%1=")").arg(key), end_pos);
            } else {
                return {text, false};
            }
        } else {
            return {text, false};
        }
    }

    std::ranges::reverse(time_list);
    for (const auto &[beginIndex, length, str] : time_list) {
        text.replace(beginIndex, length, str);
    }

    return {text, true};
}

void MainWindow::on_offsetButton_clicked() {
    ui->offsetButton->setEnabled(false); // 禁用按钮防止重复点击

    QString text = ui->TTMLTextEdit->toPlainText();
    const int offset = ui->offsetCount->value();

    ui->statusbar->showMessage(R"(开始偏移时间)");
    this->setEnabled(false);

    auto dur_offset = offsetWithKey("dur", text, offset);

    if (!std::get<1>(dur_offset)) {
        ui->statusbar->showMessage(R"(偏移 dur 失败)");
        this->setEnabled(true);
        ui->offsetButton->setEnabled(true);
        return;
    }

    auto begin_offset = offsetWithKey("begin", text, offset);

    if (!std::get<1>(begin_offset)) {
        ui->statusbar->showMessage(R"(偏移 begin 失败)");
        this->setEnabled(true);
        ui->offsetButton->setEnabled(true);
        return;
    }

    auto end_offset = offsetWithKey("end", text, offset);

    if (!std::get<1>(end_offset)) {
        ui->statusbar->showMessage(R"(偏移 end 失败)");
        this->setEnabled(true);
        ui->offsetButton->setEnabled(true);
        return;
    }

    ui->TTMLTextEdit->setPlainText(text);
    this->setEnabled(true);
    ui->offsetButton->setEnabled(true);
    ui->statusbar->showMessage(R"(时间偏移完成)");
}

std::string generateUniqueId(const size_t len) {
    // 对应 Rust 的 Alphanumeric 分布
    static constexpr char charset[] =
        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    // 对应 Rust 的 rand::rng()
    // 使用 thread_local 保证线程安全且初始化一次，性能等同于 Rust 的 ThreadRng
    thread_local std::mt19937 rng{std::random_device{}()};

    // sizeof(charset) 包含末尾的 '\0'，所以最大索引是 sizeof - 2
    thread_local std::uniform_int_distribution<size_t> dist(0, sizeof(charset) - 2);

    std::string result(len, '\0');
    for (size_t i = 0; i < len; ++i) {
        result[i] = charset[dist(rng)];
    }
    return result;
}

long long timestampMillis() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void MainWindow::on_getFilename_triggered() { // NOLINT(*-convert-member-functions-to-static)
    const auto unique_id = generateUniqueId(8);
    QApplication::clipboard()->setText(QString(R"(raw-lyrics/%1-68000793-%2.ttml)").arg(timestampMillis()).arg(QString::fromStdString(unique_id)));
}


std::tuple<QString, bool> formatTime(const QString &key, QString &text, const bool to_long, const bool to_centi, const bool to_dot) {
    auto begin_pos = text.indexOf(QString(R"(%1=")").arg(key));
    QList<std::tuple<int64_t, int64_t, QString>> time_list;

    while (begin_pos != -1) {
        auto end_pos = text.indexOf('\"', begin_pos + key.length() + 2);
        if (end_pos > begin_pos) {
            auto time_str = text.mid(begin_pos + key.length() + 2, end_pos - begin_pos - key.length() - 2);
            auto [time, success] = LyricTime::parse(time_str);
            if (success) {
                time_list.push_back(std::make_tuple(begin_pos + key.length() + 2, end_pos - begin_pos - key.length() - 2, time.toString(to_long, to_centi, to_dot)));
                begin_pos = text.indexOf(QString(R"(%1=")").arg(key), end_pos);
            } else {
                return {text, false};
            }
        } else {
            return {text, false};
        }
    }

    std::ranges::reverse(time_list);
    for (const auto &[beginIndex, length, str] : time_list) {
        text.replace(beginIndex, length, str);
    }

    return {text, true};
}

void MainWindow::on_formatTime_triggered() {
    const auto dialog = new TimeFormatDialog(this);
    dialog->setWindowTitle("设置时间格式");
    if (dialog->exec() == QDialog::Accepted) {
        auto text = ui->TTMLTextEdit->toPlainText();

        const auto dur_format = formatTime("dur", text, dialog->to_long, dialog->to_centi, dialog->to_dot);
        if (!std::get<1>(dur_format)) {
            QMessageBox::critical(this, R"(错误)", R"(格式化 dur 失败)");
            return;
        }

        const auto begin_format = formatTime("begin", text, dialog->to_long, dialog->to_centi, dialog->to_dot);
        if (!std::get<1>(begin_format)) {
            QMessageBox::critical(this, R"(错误)", R"(格式化 begin 失败)");
            return;
        }

        const auto end_format = formatTime("end", text, dialog->to_long, dialog->to_centi, dialog->to_dot);
        if (!std::get<1>(end_format)) {
            QMessageBox::critical(this, R"(错误)", R"(格式化 end 失败)");
            return;
        }

        ui->TTMLTextEdit->setPlainText(text);
    }
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

QString compressTtmlV1(QString ttml) {
    const QRegularExpression space_span_reg(R"(<span[^>]*>([\s　])</span>)");
    ttml.replace(space_span_reg, R"(\1)");

    const QRegularExpression span_space_reg(R"(([\s　])+</span>([\s　])+)");
    ttml.replace(span_space_reg, R"(</span>\2)");

    const QRegularExpression same_time_reg(R"#(<span[^>]+begin="([^"]+)"[^>]+end="\1"[^>]*>(.*?)</span>)#");
    ttml.replace(same_time_reg, R"(\2)");

    return ttml;
}

QString compressTtmlV2(QString ttml) {
    // 解析为 xml
    auto [lyric, status] = LyricObject::fromTTML(ttml);
    if (status != LyricObject::Status::Success) {
        return ttml;
    }

    return lyric.toTTML();
}

QString compressTtml(QString ttml) {
    ttml = ttml.trimmed()
    .replace(R"(" />)", R"("/>)")
    .replace(R"(" >)", R"(">)")
    .replace(R"(< )", R"(<)");

    const QRegularExpression compress_reg(R"([\n\r]+\s*)");
    ttml.replace(compress_reg, "");

    return (ttml.contains("iTunesMetadata") ? compressTtmlV2(ttml) : compressTtmlV1(ttml))
    .trimmed()
    .replace(R"(" />)", R"("/>)")
    .replace(R"(" >)", R"(">)")
    .replace(R"(< )", R"(<)");
}

bool MainWindow::parse() {
    if (this->_lyric) {
        return true;
    }

    const auto text = compressTtml(ui->TTMLTextEdit->toPlainText());

    auto [lrc, status] = LyricObject::fromTTML(text);

    if (status != LyricObject::Status::Success) {
        QMessageBox::critical(this, R"(错误)", R"(无法解析 TTML)");
        ui->statusbar->showMessage(R"(TTML 解析失败)");
        return false;
    }

    this->_lyric = std::make_unique<LyricObject>(std::move(lrc));
    return true;
}

void MainWindow::on_TTMLTextEdit_textChanged() {
    this->_lyric.reset();
    ui->countLabel->setText(QString::number(ui->TTMLTextEdit->toPlainText().length()));

    // 1. find index <amll:meta key="musicName" value="
    // 2. find index "
    // 3. text.mid(begin,end)

    // find first (?<=<amll:meta key="musicName" value=").*(?=" ?/>)
    const auto music_name_begin = ui->TTMLTextEdit->toPlainText().indexOf(R"(<amll:meta key="musicName" value=")");
    const auto music_name_end= ui->TTMLTextEdit->toPlainText().indexOf(R"(")", music_name_begin + 34);
    QString music_name = "";
    if (music_name_begin != -1 && music_name_end > music_name_begin)
        music_name = ui->TTMLTextEdit->toPlainText().mid(music_name_begin + 34, music_name_end - music_name_begin - 34);

    // find first (?<=<amll:meta key="artists" value=").*(?=" ?/>)
    const auto artists_begin = ui->TTMLTextEdit->toPlainText().indexOf(R"(<amll:meta key="artists" value=")");
    const auto artists_end= ui->TTMLTextEdit->toPlainText().indexOf(R"(")", artists_begin + 32);
    QString artists = "";
    if (artists_begin != -1 && artists_end > artists_begin)
        artists = ui->TTMLTextEdit->toPlainText().mid(artists_begin + 32, artists_end - artists_begin - 32);

    // find first (?<=<amll:meta key="album" value=").*(?=" ?/>)
    const auto albumBegin = ui->TTMLTextEdit->toPlainText().indexOf(R"(<amll:meta key="album" value=")");
    const auto albumEnd= ui->TTMLTextEdit->toPlainText().indexOf(R"(")", albumBegin + 30);
    QString album = "";
    if (albumBegin != -1 && albumEnd > albumBegin)
        album = ui->TTMLTextEdit->toPlainText().mid(albumBegin + 30, albumEnd - albumBegin - 30);

    if (not(music_name.isEmpty() or artists.isEmpty() or album.isEmpty()))
        this->setWindowTitle(QString(R"(%1 - %2 - %3)").arg(music_name).arg(artists).arg(album));
}

void MainWindow::on_toLRC_triggered() {
    // ReSharper disable once CppTooWideScopeInitStatement
    auto ok = this->parse();

    if (!ok) return;

    const auto file_path = QFileDialog::getSaveFileName(this, R"(选择文件)", this->_lyric->getTitle(R"(lrc)"),
                                                        R"(LyricObject File (*.lrc))");

    if (file_path.isEmpty()) {
        QMessageBox::warning(this, R"(警告)", R"(未选择文件)");
        return;
    }

    QStringList options{};

    options.push_back(R"(无)");
    if (this->_lyric->haveRoman()) {
        for (const auto &lang: this->_lyric->getRomaLangs()) {
            options.push_back(QString(R"(x-roman - 音译 - lang:%1)").arg(lang));
        }
    }
    if (this->_lyric->haveTrans()) {
        for (const auto &lang: this->_lyric->getTransLangs()) {
            options.push_back(QString(R"(x-trans - 翻译 - lang:%1)").arg(lang));
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

    if (!this->_lyric->getSubLangs().isEmpty() && QFileInfo::exists(ts_file_path)) {
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

    if (this->_lyric->getTransLangs().size() > 1) {
        lang = QInputDialog::getItem(this, R"(选择语言)", R"(翻译语言)", this->_lyric->getSubLangs(), 0, false, &ok);

        if (!ok) {
            QMessageBox::warning(this, R"(警告)", R"(取消选择语言)");
            return;
        }
    } elif (this->_lyric->getTransLangs().size() == 1)
        lang = this->_lyric->getTransLangs().first();

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

    if (!this->_lyric->getSubLangs().isEmpty()) {
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

    if (this->_lyric->haveTrans() && QFileInfo::exists(ts_file_path)) {
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

    QString trans_lang{};
    QString roman_lang{};

    if (this->_lyric->getTransLangs().size() == 1 and this->_lyric->getSubLangs().size() == 1) {
        trans_lang = this->_lyric->getTransLangs().first();
        roman_lang = this->_lyric->getSubLangs().first();
    } elif (this->_lyric->getTransLangs().size() > 1 or this->_lyric->getSubLangs().size() > 1) {
        const auto lang_select_dialog = new LangSelectDialog(this, this->_lyric->getTransLangs(), this->_lyric->getRomaLangs());
        if (lang_select_dialog->exec() != QDialog::Accepted) {
            QMessageBox::warning(this, R"(警告)", R"(取消选择)");
            return;
        }
        trans_lang = lang_select_dialog->getTransLang();
        roman_lang = lang_select_dialog->getRomanLang();
    }

    this->setEnabled(false);
    ui->statusbar->showMessage(R"(QRC 生成中)");

    const auto [orig_text, trans_text, roma_text] = this->_lyric->toQRC(trans_lang, roman_lang);

    const auto orig_file = new QFile(orig_file_path);
    if (!orig_file->open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, R"(错误)", QString(R"(无法打开文件：%1)").arg(orig_file_path));
        this->setEnabled(true);
        return;
    }
    orig_file->write(orig_text.toUtf8());
    orig_file->close();
    delete orig_file;

    if (not trans_text.isEmpty()) {
        const auto ts_file = new QFile(ts_file_path);
        if (!ts_file->open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::critical(this, R"(错误)", QString(R"(无法打开文件：%1)").arg(ts_file_path));
            this->setEnabled(true);
            return;
        }

        ts_file->write(trans_lang.toUtf8());
        ts_file->close();
        delete ts_file;
    }

    if (not roma_text.isEmpty()) {
        const auto roman_file = new QFile(roman_file_path);
        if (!roman_file->open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::critical(this, R"(错误)", QString(R"(无法打开文件：%1)").arg(roman_file_path));
            this->setEnabled(true);
            return;
        }

        roman_file->write(roma_text.toUtf8());
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

    if (this->_lyric->haveTrans() && QFileInfo::exists(ts_file_path)) {
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

    if (this->_lyric->getTransLangs().size() > 1) {
        lang = QInputDialog::getItem(this, R"(选择语言)", R"(翻译语言)", this->_lyric->getSubLangs(), 0, false, &ok);

        if (!ok) {
            QMessageBox::warning(this, R"(警告)", R"(取消选择语言)");
            return;
        }
    } elif (this->_lyric->getSubLangs().size() == 1)
        lang = this->_lyric->getSubLangs().first();

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

    if (not ts.isEmpty()) {
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

void MainWindow::on_toKRC_triggered() {
    // ReSharper disable once CppTooWideScopeInitStatement
    auto ok = this->parse();

    if (!ok) return;

    const auto orig_file_path = QFileDialog::getSaveFileName(this, R"(选择文件)", this->_lyric->getTitle(R"(krc)"),
                                                             R"(KuGou Music Lyrics File (*.krc))");

    if (orig_file_path.isEmpty()) {
        QMessageBox::warning(this, R"(警告)", R"(未选择文件)");
        return;
    }

    const auto file_info = QFileInfo(orig_file_path);
    QString trans_lang{};
    QString roman_lang{};

    if (this->_lyric->getTransLangs().size() == 1 and this->_lyric->getSubLangs().size() == 1) {
        trans_lang = this->_lyric->getTransLangs().first();
        roman_lang = this->_lyric->getSubLangs().first();
    } elif (this->_lyric->getTransLangs().size() > 1 or this->_lyric->getSubLangs().size() > 1) {
        const auto lang_select_dialog = new LangSelectDialog(this, this->_lyric->getTransLangs(), this->_lyric->getRomaLangs());
        if (lang_select_dialog->exec() != QDialog::Accepted) {
            QMessageBox::warning(this, R"(警告)", R"(取消选择)");
            return;
        }
        trans_lang = lang_select_dialog->getTransLang();
        roman_lang = lang_select_dialog->getRomanLang();
    }

    this->setEnabled(false);
    ui->statusbar->showMessage(R"(KRC 生成中)");

    const auto orig_text = this->_lyric->toKRC(trans_lang, roman_lang);

    const auto orig_file = new QFile(orig_file_path);
    if (!orig_file->open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, R"(错误)", QString(R"(无法打开文件：%1)").arg(orig_file_path));
        this->setEnabled(true);
        return;
    }
    orig_file->write(orig_text.toUtf8());
    orig_file->close();
    delete orig_file;

    this->setEnabled(true);
    ui->statusbar->showMessage(R"(KRC 生成完成)");
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

    const auto text = this->_lyric->toQRC("", "");
    QApplication::clipboard()->setText(std::get<0>(text));
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

QString MainWindow::s2t(const QString& val) {
    auto str = val.toStdString();
    auto buffer = opencc_convert(ot_s2t, str.c_str());
    return QString::fromUtf8(buffer);
}

QString MainWindow::t2s(const QString& val) {
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
        if (el.tagName() == "head") {
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
        if (el.tagName() == "head") {
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

    if (metas.contains("ttmlAuthorGithubLogin")) {
        buffer.push_back("### 歌词作者");

        for (const auto &autor : metas["ttmlAuthorGithubLogin"])
            buffer.push_back("- @" + autor);


        buffer.push_back("");
    }

    for (const auto &[key, alt] : preset_metas) {
        if (metas.contains(key)) {
            buffer.push_back("### " + alt);

            for (const auto &meta : metas[key])
                buffer.push_back(QString(R"(- `%1`)").arg(meta));
        }

        buffer.push_back("");
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
        buffer.push_back(QString("| **%1** | `%2` |").arg(key).arg(value));
    }

    QApplication::clipboard()->setText(buffer.join('\n'));
    ui->statusbar->showMessage("复制成功");
}

void MainWindow::on_compressButton_clicked() const {
    ui->TTMLTextEdit->setPlainText(compressTtml(ui->TTMLTextEdit->toPlainText()));
}

void MainWindow::on_fromURL_triggered()
{
    bool ok{};
    QString url = QInputDialog::getText(this, "输入URL", "请输入TTML文件的URL：", QLineEdit::EchoMode::Normal, "", &ok);

    if (!ok || url.isEmpty()) return;

    QNetworkAccessManager manager;
    const auto request = QNetworkRequest(QUrl(url));
    QNetworkReply *reply = manager.get(request);

    // 阻塞等待请求完成
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    // 连接信号（完成/超时）
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    connect(&timer, &QTimer::timeout, [&](){
        loop.quit();
        reply->abort(); // 主动终止请求
    });

    timer.start(10*1000);
    loop.exec();

    // 判断超时情况
    const bool is_timeout = !timer.isActive();
    if (is_timeout) {
        reply->deleteLater();
        ui->statusbar->showMessage("请求超时");
        return;
    }

    // 正常错误处理
    if (reply->error() != QNetworkReply::NoError) {
        reply->deleteLater();
        ui->statusbar->showMessage("请求出错");
        return;
    }

    // 处理内容...
    const QString content = QString::fromUtf8(reply->readAll());
    reply->deleteLater();

    ui->TTMLTextEdit->setPlainText(content);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void MainWindow::on_copyButton_clicked()
{
    QApplication::clipboard()->setText(ui->TTMLTextEdit->toPlainText());
}
