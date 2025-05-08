#ifndef MPLAINTEXTEDIT_H
#define MPLAINTEXTEDIT_H

#include <QPlainTextEdit>

class MPlainTextEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit MPlainTextEdit(QWidget *parent = nullptr);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

    signals:

    };

#endif // MPLAINTEXTEDIT_H
