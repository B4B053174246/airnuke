#ifndef AIRUTIL_H
#define AIRUTIL_H
#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QStringList>

bool isStringValid(QString str);
bool fileorfolderExists(QString path);
bool fileExists(QString path);
bool checkDependencies(const QStringList filepaths);


#endif // AIRUTIL_H
