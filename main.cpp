
#include "airnuke.h"





int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    airnuke obj;

    QObject::connect(&obj, SIGNAL(finished()),
             &app, SLOT(quit()));
    QObject::connect(&app, SIGNAL(aboutToQuit()),
             &obj, SLOT(aboutToQuitApp()));

    QTimer::singleShot(10,&obj, SLOT(run()));




    return app.exec();
}
