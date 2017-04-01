#include "airutil.h"


bool isStringValid(QString str)
{
    if (str.isNull())
    {
        return false;
    }

    if (str.isEmpty())
    {
        return false;
    }

    return true;
}

bool fileorfolderExists(QString path)
{
    if(isStringValid(path) == false)
    {
        return false;
    }

    QFileInfo info(path);

    return info.exists() || info.isDir() || info.isSymLink();
}



bool fileExists(QString path) {

    if(isStringValid(path) == false)
    {
        return false;
    }

    QFileInfo check_file(path);

    //Check if exists
    if (check_file.exists() == false)
    {
        return false;
    }

    //Check if file
    if (check_file.isFile() == false)
    {
        return false;
    }

    return true;
}

bool checkDependencies(const QStringList filepaths)
{
    foreach (const QString &str, filepaths)
    {
        if (fileExists(str) == false)
        {
            return false;
        }
    }
    return true;
}
