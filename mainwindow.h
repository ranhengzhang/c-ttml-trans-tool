#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "LyricObject.h"
#include "opencc.h"

QT_BEGIN_NAMESPACE

namespace Ui {
    class MainWindow;
}

QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    // ReSharper disable once CppNonExplicitConvertingConstructor
    MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

private slots:
    void on_TTMLTextEdit_textChanged();

    void on_offsetButton_clicked();

    void on_getFilename_triggered();

    void on_formatTime_triggered();

    void on_fromFile_triggered();

    void on_fromClipboard_triggered();

    void on_toTTML_triggered();

    void on_toASS_triggered();

    void on_toLRC_triggered();

    void on_toSPL_triggered();

    void on_toLYS_triggered();

    void on_toQRC_triggered();

    void on_toYRC_triggered();

    void on_toKRC_triggered();

    void on_toTXT_triggered();

    void on_fromLYS_triggered();

    void on_fromQRC_triggered();

    void on_fromYRC_triggered();

    void on_copyTTML_triggered();

    void on_copyASS_triggered();

    void on_copyLRC_triggered();

    void on_copySPL_triggered();

    void on_copyLYS_triggered();

    void on_copyQRC_triggered();

    void on_copyYRC_triggered();

    void on_copyTXT_triggered();

    void on_actions2t_triggered();

    void on_actiont2s_triggered();

    void on_actionPreset_triggered();

    void on_actionExtra_triggered();

    void on_compressButton_clicked() const;

    void on_fromURL_triggered();

    void on_copyButton_clicked();

private:
    [[nodiscard]] QString t2s(QString val);

    [[nodiscard]] QString s2t(QString val);

    void node_t2s(QDomNode &node);

    void node_s2t(QDomNode &node);

    OpenCCHandle *ot_t2s{nullptr};

    OpenCCHandle *ot_s2t{nullptr};

    bool parse();

    Ui::MainWindow *ui;

    std::unique_ptr<LyricObject> _lyric{};

    static QList<std::tuple<QString, QString, QString>> presetMetas;
};
#endif // MAINWINDOW_H
