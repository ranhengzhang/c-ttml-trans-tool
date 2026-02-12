#include "src/dialogs/MainWindow.hpp"

#include <QApplication>
#include <QMessageBox>
#include <windows.h> // 必须引入 Windows 原生头文件

int main(int argc, char *argv[])
{
    // 1. 创建互斥量。建议使用唯一的字符串，如 "Global\" + 你的程序名
    // "Global\" 前缀确保在多用户/远程桌面环境下也是唯一的
    HANDLE hMutex = CreateMutexA(NULL, FALSE, "Global\\TTML_TRANS_TOOL_SINGLE_INSTANCE_MUTEX");

    // 2. 检查互斥量是否已存在
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        // 如果已存在，说明已有实例在运行
        // 提示：由于此时 QApplication 还没完全运行，建议使用系统弹窗或简单的 QMessageBox
        QApplication a(argc, argv); // 为了显示中文弹窗，临时初始化 a
        QMessageBox::warning(nullptr, "提示", "程序已经在运行中，请勿重复启动。");

        // 释放句柄并退出
        if (hMutex) {
            CloseHandle(hMutex);
        }
        return 0;
    }

    QApplication a(argc, argv);

    MainWindow w;
    w.setWindowIcon(QIcon(R"(://winicon.ico)"));
    w.setWindowTitle(R"(TTML TRANS TOOL)");
    w.show();

    int result = a.exec();

    // 3. 程序正常退出时，关闭并释放互斥量句柄
    if (hMutex) {
        CloseHandle(hMutex);
    }

    return result;
}
