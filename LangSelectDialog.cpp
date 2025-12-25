#include "LangSelectDialog.h"
#include "ui_LangSelectDialog.h"

LangSelectDialog::LangSelectDialog(QWidget *parent, QStringList ts_lang_s, QStringList roma_lang_s) :
    QDialog(parent),
    ui(new Ui::LangSelectDialog),
    _ts_lang_s(ts_lang_s),
    _roma_lang_s(roma_lang_s)
{
    ui->setupUi(this);

    ui->tsComboBox->addItems(ts_lang_s);
    ui->romaComboBox->addItems(roma_lang_s);
}

LangSelectDialog::~LangSelectDialog()
{
    delete ui;
}

QString LangSelectDialog::getTransLang() const {
    return ui->tsComboBox->currentText();
}

QString LangSelectDialog::getRomanLang() const {
    return ui->romaComboBox->currentText();
}
