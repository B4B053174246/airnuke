#ifndef AIRODUMP_H
#define AIRODUMP_H


#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QStringList>
#include <QProcess>
#include <QDateTime>
#include <QThread>
#include <QTextStream>
#include <QDateTime>
#include <QVariantList>
#include <QFileInfo>
#include <QDir>
#include <QProcess>
#include <QHash>
#include <QThread>
#include <QPair>
#include "airutil.h"

#define AIRODUMPTRIES 4
#define AIRODUMPPROCESSTIME 3000
#define NOMODE 0
#define BSSIDINFOMODE 1
#define STATIONMODE 2
#define BSSIDLENGTH 17

typedef struct airodumpdata
{
    QProcess *process;
    QString path;
    QString band;
} Airodumpdata;

typedef struct BSSIDdata
{
    QDateTime firstseen;
    QDateTime lastseen;
    unsigned int channel;
    QString speed;
    QString privacy;
    QString cipher;
    QString authentication;
    int power;
    unsigned long beacons;
    unsigned long iv;
} BSSIDdata;


typedef struct BSSIDMACdata
{
    QDateTime firstseen;
    QDateTime lastseen;
    int power;
    unsigned long packets;
} BSSIDMACdata;




bool startAirodump(QStringList args,QString band,QString filepath, QString interface,QHash<QString, Airodumpdata> *airodumpMap);
bool stopAirodump(QString interface,QHash<QString, Airodumpdata> *airodumpMap);
bool airodumpALL(QStringList args,QString band,QString dir, QStringList interface,QHash<QString, Airodumpdata> *airodumpMap);
bool stopALLAirodump(QHash<QString, Airodumpdata> *airodumpMap);
bool isCSVValid(QString filepath);
bool getChannelfromCSV(QString filepath, QString bssid, QString * channelnum);
bool findAllCSV(QString filepath,QStringList *files);
bool getBSSIDinfo(QString filepath, BSSIDdata *BSSIDinfo,QHash<QString, BSSIDMACdata> *map);
bool mergeBSSIDinfo(QStringList CSVfiles,QString currentdir, BSSIDdata *BSSIDinfo, QHash<QString, BSSIDMACdata> *map);

#endif // AIRODUMP_H
