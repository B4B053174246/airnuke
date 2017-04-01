#include "log.h"

bool openLog(QString filepath,JSONdata* jdata)
{
    if (isStringValid(filepath) == false)
    {
        jdata->log = false;
        return false;
    }


    jdata->logfile = new QFile(filepath);
    if (jdata->logfile->open(QIODevice::WriteOnly | QIODevice::Append) == false)
    {
        jdata->log = false;
        return false;
    }

    jdata->log = true;

    return true;
}

void logInfo(JSONdata jdata,QString message)
{
    if (jdata.verbose == true)
    {
        qInfo().noquote() << message;
    }

    if (jdata.log == true)
    {
        QTextStream ts(jdata.logfile);
        ts << message << endl;
    }

}

bool closeLog(JSONdata* jdata)
{
     if (jdata->log  == false)
     {
         return false;
     }

     jdata->logfile->close();
     jdata->log = false;
     return true;
}



