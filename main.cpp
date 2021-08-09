#include"mydouyin.h"
#include"ckernel.h"
#include <QApplication>

#undef main
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    CKernel::GetInstance();

    return a.exec();
}
