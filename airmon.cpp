#include "airmon.h"



bool airmonCheck()
{
    QProcess process;
    process.start("airmon-ng");
    process.waitForFinished(-1);

    QString stdout = process.readAllStandardOutput();

    return stdout.contains("Interface", Qt::CaseSensitive);
}


bool isMonitorMode(QString iname)
{
    if(isStringValid(iname) == false)
    {
        return false;
    }

    QProcess process;
    process.start("iwconfig "+iname);
    process.waitForFinished(-1);

    QString stdout = process.readAllStandardOutput();

    return stdout.contains("Mode:Monitor", Qt::CaseSensitive);
}
