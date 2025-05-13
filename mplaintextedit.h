#ifndef MPLAINTEXTEDIT_H
#define MPLAINTEXTEDIT_H

#include <QPlainTextEdit>

class MPlainTextEdit final : public QPlainTextEdit {
    Q_OBJECT
public:
    explicit MPlainTextEdit(QWidget *parent = nullptr);
    void setTextProgrammatically(const QString &text);

    signals:
        void textEdited();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    bool _isProgrammaticChange{};

private slots:
    void onTextChanged();
};

#endif // MPLAINTEXTEDIT_H
