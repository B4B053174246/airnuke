#ifndef AIREPLAY_H
#define AIREPLAY_H


#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QStringList>
#include <QProcess>
#include <QDateTime>
#include <QThread>
#include <QDateTime>
#include <QVariantList>
#include <QFileInfo>
#include <QDir>
#include <QProcess>
#include <QHash>
#include <QThread>
#include <QPair>
#include "airutil.h"

#define AIREPLAYTRIES 4
#define AIREPLAYPROCESSTIME 3000


typedef struct aireplaydata
{
    QProcess *process;
    QDateTime lastseen;
    QString iname;
} Aireplaydata;



bool startAireplay(QStringList args, QString MACaddr, QString interface, QHash<QString, Aireplaydata> *aireplayMap);
bool stopAireplay(QString MACaddr,QHash<QString, Aireplaydata> *aireplayMap);
bool stopALLAireplay(QHash<QString, Aireplaydata> *aireplayMap);
bool removeOldAireplays(QList<QPair<QString, QDateTime>> inputData,QHash<QString, Aireplaydata> *aireplayMap );
bool updateAireplays(QList<QPair<QString, QDateTime>> inputData, QString BSSID, QStringList interfaces, QHash<QString, Aireplaydata> *aireplayMap );
bool interfaceCount(QHash<QString, Aireplaydata> aireplayMap,QStringList interfaces, QHash<QString, unsigned int> *sorted);
bool leastUsedInterface(QHash<QString, Aireplaydata> aireplayMap,QStringList interfaces,QString *interface );

#endif // AIREPLAY_H
