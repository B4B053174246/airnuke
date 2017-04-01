#ifndef JSONUTIL_H
#define JSONUTIL_H
#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QStringList>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>
#include <QVariantList>
#include "airutil.h"

bool readJSONfile(QString path,QJsonDocument *jsondoc);
bool objectToValue(QJsonObject jsonobj,QString key, QJsonValue *jvalue);
bool getStringfromObject(QJsonObject jsonobj, QString key, QString *string);
bool getBoolfromObject(QJsonObject jsonobj, QString key, bool *value);
bool getStringListfromObject(QJsonObject jsonobj, QString key, QStringList *list);


#endif // JSONUTIL_H
