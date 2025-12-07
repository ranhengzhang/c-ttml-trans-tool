#include "timeformatdialog.h"
#include "ui_timeformatdialog.h"

TimeFormatDialog::TimeFormatDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TimeFormatDialog)
{
    ui->setupUi(this);

    this->refreshDisplay();
}

TimeFormatDialog::~TimeFormatDialog()
{
    delete ui;
}

void TimeFormatDialog::on_prefix_currentTextChanged(const QString &arg1)
{
    this->to_long = arg1 == "完整";
    this->refreshDisplay();
}


void TimeFormatDialog::on_suffix_currentTextChanged(const QString &arg1)
{
    this->to_centi = arg1 == "厘秒";
    this->refreshDisplay();
}


void TimeFormatDialog::on_symbol_currentTextChanged(const QString &arg1)
{
    this->to_dot = arg1 == "点号";
    this->refreshDisplay();
}

void TimeFormatDialog::refreshDisplay() {
    ui->display->setText(QString("%1%2%3").arg(this->to_long ? "00:00" : "0").arg(this->to_dot ? "." : ":").arg(this->to_centi ? "00" : "000"));
}
