#include "jsonutil.h"

bool readJSONfile(QString path,QJsonDocument *jsondoc)
{
    if(isStringValid(path) == false)
    {
        return false;
    }

    if(jsondoc == NULL)
    {
        return false;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return false;
    }

    QJsonParseError error;
    *jsondoc = QJsonDocument::fromJson(file.readAll(),&error);
    if (error.error != QJsonParseError::NoError)
    {
        return false;
    }
    file.close();

    return true;
}

bool objectToValue(QJsonObject jsonobj,QString key, QJsonValue *jvalue)
{
    if(jsonobj.isEmpty() == true)
    {
        return false;
    }

    if(isStringValid(key) == false)
    {
        return false;
    }

    if(jvalue == NULL)
    {
        return false;
    }

    QJsonValue temp = jsonobj.value(key);
    if (temp.isNull() == true)
    {
        return false;
    }
    if (temp.isUndefined() == true)
    {
        return false;
    }
    (*jvalue) = temp;

    return true;
}

bool getStringfromObject(QJsonObject jsonobj,QString key,QString *string)
{
    QJsonValue jvalue;
    bool isValid = objectToValue(jsonobj,key, &jvalue);
    if (isValid == false)
    {
        return false;
    }

    if (jvalue.isString() == false)
    {
        return false;
    }
    QString tempstr = jvalue.toString();

    if(isStringValid(tempstr) == false)
    {
        return false;
    }

    (*string) = tempstr;

    return true;
}

bool getBoolfromObject(QJsonObject jsonobj, QString key, bool *value)
{
    QJsonValue jvalue;
    bool isValid = objectToValue(jsonobj,key, &jvalue);
    if (isValid == false)
    {
        return false;
    }

    if (jvalue.isBool() == false)
    {
        return false;
    }
    (*value) = jvalue.toBool();

    return true;
}

bool getStringListfromObject(QJsonObject jsonobj, QString key, QStringList *list)
{
    QJsonValue jvalue;
    bool isValid = objectToValue(jsonobj,key, &jvalue);
    if (isValid == false)
    {
        return false;
    }

    if (jvalue.isArray() == false)
    {
        return false;
    }

    QVariant jarray = jvalue.toVariant();
    if (jarray.isNull() == true)
    {
        return false;
    }

    if (jarray.isValid() == false)
    {
        return false;
    }

    QStringList qlist = jarray.toStringList();
    if (qlist.isEmpty() == true)
    {
        return false;
    }

    *list = qlist;

    return true;
}
