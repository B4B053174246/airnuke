#include "airodump.h"


bool startAirodump(QStringList args,QString band,QString filepath, QString interface,QHash<QString, Airodumpdata> *airodumpMap)
{
    //Create process
    QProcess *proc = new QProcess;

    //Band
    if (band.isEmpty() == false)
    {
        args << "--band" << band;
    }

    //Other info
    args << "--output-format" << "csv" << "--update" << "10000000" << "--write-interval" << "1" << "-w" << filepath << interface;

    unsigned int i = 0;
    //Start
    while (i < AIRODUMPTRIES)
    {
        //Delete files
        QFile::remove(filepath+"-01.cap");
        QFile::remove(filepath+"-01.csv");
        QFile::remove(filepath+"-01.kismet.csv");
        QFile::remove(filepath+"-01.kismet.netxml");

        //Start
        proc->start("airodump-ng", args ,QIODevice::WriteOnly);
        proc->closeReadChannel(QProcess::StandardOutput);
        proc->closeReadChannel(QProcess::StandardError);

        proc->waitForStarted(AIRODUMPPROCESSTIME);

        QThread::msleep(AIRODUMPPROCESSTIME);


        if (isCSVValid(filepath+"-01.csv") == true)
        {
            break;
        }

        i++;
        proc->kill();
        proc->waitForFinished(AIRODUMPPROCESSTIME);
    }

    if (i == AIRODUMPTRIES)
    {
        proc->deleteLater();
        return false;
    }


    //Append to map
    Airodumpdata data;
    data.band = band;
    data.path = filepath;
    data.process = proc;
    airodumpMap->insert(interface,data);
    return true;
}

bool stopAirodump(QString interface,QHash<QString, Airodumpdata> *airodumpMap)
{
    Airodumpdata data = airodumpMap->value(interface);
    QProcess *proc = data.process;
    proc->terminate();
    bool terminated = proc->waitForFinished(AIRODUMPPROCESSTIME);
    if (terminated == false)
    {
        unsigned int i = 0;
        while (i < AIRODUMPTRIES)
        {
            proc->kill();
            terminated = proc->waitForFinished(AIRODUMPPROCESSTIME);
            if (terminated == true)
            {
                break;
            }
        }
        if (i == AIRODUMPTRIES)
        {
            return false;
        }
    }


    data.process->deleteLater();
    airodumpMap->remove(interface);
    return true;
}


bool airodumpALL(QStringList args,QString band,QString dir, QStringList interfaces,QHash<QString, Airodumpdata> *airodumpMap)
{
    int i;
    bool executed = true;
    for(i=0;i<interfaces.length();++i)
    {
        bool isValid = startAirodump(args,band,dir+"dump"+QString::number(i),interfaces.at(i),airodumpMap);
        if (isValid == false)
        {
            executed = false;
        }
    }
    return executed;
}

bool stopALLAirodump(QHash<QString, Airodumpdata> *airodumpMap)
{
    QHashIterator<QString, Airodumpdata> info(*airodumpMap);
    bool executed = true;
    while (info.hasNext()) {
        info.next();
        bool isValid = stopAirodump(info.key(),airodumpMap);
        if (isValid == false)
        {
            executed = false;
        }
    }
    return executed;
}



bool isCSVValid(QString filepath)
{
    //Check file
    if (fileExists(filepath) == false)
    {
            return false;
    }

    //Check data
    BSSIDdata BSSIDinfotmp;
    QHash<QString, BSSIDMACdata> maptmp;
    bool isValid = getBSSIDinfo(filepath,&BSSIDinfotmp,&maptmp);
    if (isValid == false)
    {
        return false;
    }


    if (BSSIDinfotmp.firstseen.isNull() == true)
    {
        return false;
    }
    return true;
}


bool getChannelfromCSV(QString filepath, QString bssid, QString * channelnum)
{
    QFile inputFile(filepath);
    if (inputFile.open(QIODevice::ReadOnly) == false)
    {
        return false;
    }

    QTextStream in(&inputFile);
    while (!in.atEnd())
    {
        QString line = in.readLine();

        if (line.length() < BSSIDLENGTH)
        {
            continue;
        }

        QString lineBSSID = line.left(BSSIDLENGTH);
        if (lineBSSID == bssid)
        {
            QStringList data = line.split(',');
            QString channelnumber = data.at(3).trimmed();
            if (channelnumber.toInt() < 1)
            {
                continue;
            }
            *channelnum = channelnumber;
            return true;
        }
    }
    inputFile.close();

    return false;
}

bool findAllCSV(QString filepath,QStringList *files)
{
    QDir dir(filepath);
    dir.setNameFilters(QStringList() << "*-01.csv");
    dir.setFilter(QDir::Files);
    bool isValid = false;
    foreach(QString dirFile, dir.entryList())
    {
        files->append(dirFile);
        isValid = true;
    }
    return isValid;
}


bool getBSSIDinfo(QString filepath,BSSIDdata *BSSIDinfo,QHash<QString, BSSIDMACdata> *map)
{
    if (fileExists(filepath) == false)
    {
        return false;
    }


    QFile inputFile(filepath);
    if (inputFile.open(QIODevice::ReadOnly) == false)
    {
        return false;
    }

    QTextStream in(&inputFile);
    char mode = NOMODE;
    bool isValid = false;
    while (!in.atEnd())
    {
        QString line = in.readLine();

        if (line.length() < BSSIDLENGTH)
        {
            continue;
        }

        //Read headers
        if (line.left(5) == "BSSID")
        {
            mode = BSSIDINFOMODE;
            continue;
        }
        else if (line.left(11) == "Station MAC")
        {
            mode = STATIONMODE;
            continue;
        }

        QStringList data = line.split(',');
        QStringList trimmedList;
        foreach (const QString &str, data)
        {
            trimmedList << str.trimmed();
        }

        if (mode == BSSIDINFOMODE)
        {
            BSSIDinfo->firstseen = QDateTime::fromString( trimmedList.at(1), "yyyy-MM-dd HH:mm:ss");
            BSSIDinfo->lastseen = QDateTime::fromString(trimmedList.at(2), "yyyy-MM-dd HH:mm:ss");
            BSSIDinfo->channel = trimmedList.at(3).toUInt();
            BSSIDinfo->speed = trimmedList.at(4);
            BSSIDinfo->privacy = trimmedList.at(5);
            BSSIDinfo->cipher = trimmedList.at(6);
            BSSIDinfo->authentication = trimmedList.at(7);
            BSSIDinfo->power = trimmedList.at(8).toInt();
            BSSIDinfo->beacons = trimmedList.at(9).toULong();
            BSSIDinfo->iv = trimmedList.at(10).toULong();
            isValid = true;
        }
        else if (mode == STATIONMODE)
        {
            QString MACaddr = trimmedList.at(0);
            BSSIDMACdata MACdata;
            MACdata.lastseen = QDateTime::fromString(trimmedList.at(2), "yyyy-MM-dd HH:mm:ss");

            if (map->contains(MACaddr))
            {
                BSSIDMACdata prevMACdata = map->value(MACaddr);
                if (prevMACdata.lastseen >= MACdata.lastseen)
                {
                    continue;
                }
            }

            MACdata.firstseen = QDateTime::fromString(trimmedList.at(1), "yyyy-MM-dd HH:mm:ss");
            MACdata.power = trimmedList.at(3).toInt();
            MACdata.packets = trimmedList.at(4).toULong();
            map->insert(MACaddr,MACdata);
        }
    }
    inputFile.close();
    return isValid;
}



bool mergeBSSIDinfo(QStringList CSVfiles,QString currentdir, BSSIDdata *BSSIDinfo, QHash<QString, BSSIDMACdata> *map)
{
    unsigned int length = CSVfiles.length();
    BSSIDinfo->power = 0;
    BSSIDinfo->beacons = 0;
    BSSIDinfo->iv = 0;

    BSSIDdata currentBSSIDinfo;
    bool isValid;
    foreach (const QString &str, CSVfiles)
    {
        QHash<QString, BSSIDMACdata> tempMap;
        isValid = getBSSIDinfo(currentdir+str,&currentBSSIDinfo,&tempMap);
        if (isValid == false)
        {
            continue;
        }

        //Insert hash into other hash
        QHashIterator<QString, BSSIDMACdata> info(tempMap);
        while (info.hasNext()) {
            info.next();
            map->insert(info.key(),info.value());
        }

        //Average
        BSSIDinfo->power += currentBSSIDinfo.power/((int)length);
        BSSIDinfo->beacons += currentBSSIDinfo.beacons/((unsigned long)length);
        BSSIDinfo->iv += currentBSSIDinfo.iv/((unsigned long)length);
    }

    if (BSSIDinfo->beacons == 0)
    {
        return false;
    }

    BSSIDinfo->firstseen = currentBSSIDinfo.firstseen;
    BSSIDinfo->lastseen = currentBSSIDinfo.lastseen;
    BSSIDinfo->channel = currentBSSIDinfo.channel;
    BSSIDinfo->speed = currentBSSIDinfo.speed;
    BSSIDinfo->privacy = currentBSSIDinfo.privacy;
    BSSIDinfo->cipher = currentBSSIDinfo.cipher;
    BSSIDinfo->authentication = currentBSSIDinfo.authentication;

    return true;
}

