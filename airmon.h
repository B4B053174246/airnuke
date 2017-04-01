#ifndef AIRMON_H
#define AIRMON_H

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QStringList>
#include <QProcess>
#include "airutil.h"

bool airmonCheck();
bool isMonitorMode(QString iname);

#endif // AIRMON_H
