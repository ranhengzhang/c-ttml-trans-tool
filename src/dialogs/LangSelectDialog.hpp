#ifndef LANGSELECTDIALOG_H
#define LANGSELECTDIALOG_H

#include <QDialog>

namespace Ui {
class LangSelectDialog;
}

class LangSelectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LangSelectDialog(QWidget *parent = nullptr, QStringList ts_lang_s = QStringList(), QStringList roma_lang_s = QStringList());
    ~LangSelectDialog() override;

    QString getTransLang() const;

    QString getRomanLang() const;

private:
    Ui::LangSelectDialog *ui;

    QStringList _ts_lang_s;
    QStringList _roma_lang_s;
};

#endif // LANGSELECTDIALOG_H
