#ifndef TIMEFORMATDIALOG_H
#define TIMEFORMATDIALOG_H

#include <QDialog>

namespace Ui {
class TimeFormatDialog;
}

class TimeFormatDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TimeFormatDialog(QWidget *parent = nullptr);

    ~TimeFormatDialog();

    bool to_long{false};

    bool to_centi{false};

    bool to_dot{false};

private slots:
    void on_prefix_currentTextChanged(const QString &arg1);

    void on_suffix_currentTextChanged(const QString &arg1);

    void on_symbol_currentTextChanged(const QString &arg1);

private:
    void refreshDisplay();

    Ui::TimeFormatDialog *ui;
};

#endif // TIMEFORMATDIALOG_H
