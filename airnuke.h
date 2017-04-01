#ifndef AIRNUKE_H
#define AIRNUKE_H
#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QStringList>
#include <QDebug>
#include <QTimer>
#include <QObject>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>
#include <QDateTime>
#include <QVariantList>
#include <QFileInfo>
#include <QDir>
#include <QProcess>
#include <QHash>
#include <QThread>
#include <QPair>
#include <QTextStream>

#include "airmon.h"
#include "airutil.h"
#include "jsonutil.h"
#include "airodump.h"
#include "aireplay.h"



#define PATHSTR "Aircrack suite not installed"
#define AIRNOTWORKSTR "Aircrack suite not working"
#define CMDHELPSTR "airnuke [JSON file]"
#define JSONHELP "Not a JSON file"
#define INVALIDJSON "Invalid JSON structure"
#define INITSTR "Unable to init"
#define INVALIDLOG "Unable to open log file"

#define DIRSTR "dir"
#define BSSIDSTR "BSSID"
#define INAMESTR "interfaces"
#define BANDSTR "band"
#define RUNTIMEDSTR "runtime"
#define VERBOSESTR "verbose"
#define LOGSTR "logfile"

#define MAPSIZE 30
#define CHANNELLISTENTIME 7000
#define PROCESSTIME 3000
#define AIREPLAYSPERCARD 4

#define TIMEFORMAT "yyyy-MM-dd hh:mm:ss"


typedef struct JSONdata
{
    QString dir;
    QString BSSID;
    QStringList interfaces;
    QString band;
    QDateTime runtime;
    bool verbose;
    bool log;
    QFile* logfile;
} JSONdata;






class airnuke: public QObject {
    Q_OBJECT
public:
    explicit airnuke(QObject *parent = 0);
    void quit();

    const QStringList binpaths{"/bin/airmon-ng",
                              "/bin/airodump-ng",
                              "/bin/aireplay-ng"
                             };

private:
    QCoreApplication *app;

    bool checkArgs(QCoreApplication *a);
    bool parseJSON(QJsonDocument jsondoc, JSONdata *curjdata);
    bool validateJSON(JSONdata curjdata);
    bool sortByTime(QHash<QString, BSSIDMACdata> map,QList<QPair<QString, QDateTime>> *sortedMap);
    bool takeNitems(unsigned int N,QList<QPair<QString, QDateTime>> *sortedMap);
    bool loop();
    bool init(JSONdata curjdata);
    bool cleanup();

    QHash<QString, Aireplaydata> aireplayMap;
    QHash<QString, Airodumpdata> airodumpMap;
    JSONdata jdata;

signals:
    void finished();

public slots:
    void run();
    void aboutToQuitApp();


};




#endif // AIRNUKE_H
