#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "lyric.h"

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

    void on_fromFile_triggered();

    void on_fromClipboard_triggered();

    void on_toTTML_triggered();

    void on_toASS_triggered();

    void on_toLRC_triggered();

    void on_toLYS_triggered();

    void on_toSPL_triggered();

    void on_toQRC_triggered();

    void on_toYRC_triggered();

    void on_toTXT_triggered();

    void on_fromLYS_triggered();

    void on_fromQRC_triggered();

    void on_fromYRC_triggered();

private:
    Ui::MainWindow *ui;

    std::unique_ptr<Lyric> _lyric{};

    bool parse();
};
#endif // MAINWINDOW_H
