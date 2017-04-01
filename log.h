#ifndef LOG_H
#define LOG_H


#include "airnuke.h"
bool openLog(QString filepath,JSONdata* jdata);
void logInfo(JSONdata jdata, QString message);
bool closeLog(JSONdata* jdata);


#endif // LOG_H
