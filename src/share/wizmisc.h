#ifndef WIZMISC_H
#define WIZMISC_H

#include <stdint.h>

#include <QIcon>
#include <QBuffer>
#include <QByteArray>

#include "wizqthelper.h"
#include "wizmd5.h"

#define WIZNOTE_OBSOLETE


class IWizGlobal : public QObject
{
    Q_OBJECT

public:
    static IWizGlobal* instance();

    QString version() const { return m_strVersion; }
    void setVersion(const QString& strVersion) { m_strVersion = strVersion; }

    QBuffer* bufferLog() { return &m_bufferLog; }
    CString GetTempPath();
    void WriteLog(const CString& str);
    void WriteDebugLog(const CString& str);

private:
    IWizGlobal();
    static IWizGlobal* m_pInstance;

    QString m_strVersion;
    QBuffer m_bufferLog;
};

IWizGlobal* WizGlobal();

QString WizGetTimeStamp();
COleDateTime WizGetCurrentTime();
bool WizStringToDateTime(const QString& str, COleDateTime& t, QString& strError);
COleDateTime WizStringToDateTime(const CString& str);
COLORREF WizStringToColor(const CString& str);
QColor WizStringToColor2(const CString& str);

std::string WizBSTR2UTF8(const CString& str);

CString WizFormatString0(const CString& str);
CString WizFormatString1(const CString& strFormat, const CString& strParam1);
CString WizFormatString2(const CString& strFormat, const CString& strParam1, const CString& strParam2);
CString WizFormatString3(const CString& strFormat, const CString& strParam1, const CString& strParam2, const CString& strParam3);
CString WizFormatString4(const CString& strFormat, const CString& strParam1, const CString& strParam2, const CString& strParam3, const CString& strParam4);
CString WizFormatString5(const CString& strFormat, const CString& strParam1, const CString& strParam2, const CString& strParam3, const CString& strParam4, const CString& strParam5);
CString WizFormatString6(const CString& strFormat, const CString& strParam1, const CString& strParam2, const CString& strParam3, const CString& strParam4, const CString& strParam5, const CString& strParam6);
CString WizFormatString7(const CString& strFormat, const CString& strParam1, const CString& strParam2, const CString& strParam3, const CString& strParam4, const CString& strParam5, const CString& strParam6, const CString& strParam7);
CString WizFormatString8(const CString& strFormat, const CString& strParam1, const CString& strParam2, const CString& strParam3, const CString& strParam4, const CString& strParam5, const CString& strParam6, const CString& strParam7, const CString& strParam8);
CString WizFormatInt(__int64 n);

time_t WizTimeGetTimeT(const COleDateTime& t);

CString WizIntToStr(int n);

BOOL WizSplitTextToArray(const CString& strText, QChar ch, CWizStdStringArray& arrayResult);
BOOL WizSplitTextToArray(CString strText, const CString& strSplitterText, BOOL bMatchCase, CWizStdStringArray& arrayResult);
void WizStringArrayToText(const CWizStdStringArray& arrayText, CString& strText, const CString& strSplitter);
int WizFindInArray(const CWizStdStringArray& arrayText, const CString& strFind);
int WizFindInArrayNoCase(const CWizStdStringArray& arrayText, const CString& strFind);
void WizStringArrayEraseEmptyLine(CWizStdStringArray& arrayText);
void WizStringArrayRemoveMultiElement(CWizStdStringArray& arrayText);
void WizStringArrayRemoveMultiElementNoCase(CWizStdStringArray& arrayText);


CString WizStringArrayGetValue(const CWizStdStringArray& arrayText, const CString& valueName);
void WizCommandLineToStringArray(const CString& commandLine, CWizStdStringArray& arrayLine);
CString WizGetCommandLineValue(const CString& strCommandLine, const CString& strKey);

BOOL WizStringSimpleSplit(const CString& str, char ch, CString& strLeft, CString& strRight);

CString WizDateToLocalString(const COleDateTime& t);

QString WizGetAppPath();
QString WizGetAppFileName();
QString WizGetResourcesPath();
QString WizGetDataStorePath();
QString WizGetUpgradePath();
CString WizGetSettingsFileName();
QString WizGetLogFileName();
QString WizGetLocaleFileName(const QString& strLocale);
QString WizGetQtLocaleFileName(const QString& strLocale);
void WizGetTranslatedLocales(QStringList& locales);
QString WizGetTranslatedLocaleDisplayName(int index);


qint64 WizGetFileSize(const CString& strFileName);
QString WizGetFileSizeHumanReadalbe(const QString& strFileName);

void WizPathAddBackslash(QString& strPath);
void WizPathRemoveBackslash(CString& strPath);
CString WizPathAddBackslash2(const CString& strPath);
CString WizPathRemoveBackslash2(const CString& strPath);
void WizEnsurePathExists(const CString& strPath);
void WizEnsureFileExists(const QString& strFileName);

CString WizExtractFilePath(const CString& strFileName);
CString WizExtractFileName(const CString& strFileName);
CString WizExtractFileTitle(const CString& strFileName);
CString WizExtractTitleTemplate(const CString& strFileName);
CString WizExtractFileExt(const CString& strFileName);

#define EF_INCLUDEHIDDEN			0x01
#define EF_INCLUDESUBDIR			0x02

void WizEnumFiles(const QString& strPath, const QString& strExts, CWizStdStringArray& arrayFiles, UINT uFlags);
void WizEnumFolders(const QString& strPath, CWizStdStringArray& arrayFolders, UINT uFlags);
QString WizFolderNameByPath(const QString& strPath);

BOOL WizCopyFile(const CString& strSrcFileName, const CString& strDestFileName, BOOL bFailIfExists);
void WizGetNextFileName(CString& strFileName);

QString WizEncryptPassword(const QString& strPassword);
QString WizDecryptPassword(const QString& strEncryptedText);


bool WizLoadUnicodeTextFromFile(const QString& strFileName, QString& steText);
bool WizSaveUnicodeTextToUtf16File(const QString& strFileName, const QString& strText);
bool WizSaveUnicodeTextToUtf8File(const QString& strFileName, const QString& strText);

CString WizDateTimeToIso8601String(const COleDateTime& t);
BOOL WizIso8601StringToDateTime(CString str, COleDateTime& t, CString& strError);
CString WizDateTimeToString(const COleDateTime& t);
CString WizStringToSQL(const CString& str);
CString WizTimeToSQL(const COleDateTime& t);
CString WizColorToString(COLORREF cr);
CString WizColorToString(const QColor& cr);
CString WizColorToSQL(COLORREF cr);
CString WizColorToSQL(const QColor& cr);

intptr_t WizStrStrI_Pos(const CString& str, const CString& Find, int nStart = 0);

CString WizInt64ToStr(__int64 n);

CString WizGenGUIDLowerCaseLetterOnly();


QString WizGetComputerName();

#define TOLOG(x)                        WizGlobal()->WriteLog(x)
#define TOLOG1(x, p1)                   WizGlobal()->WriteLog(WizFormatString1(x, p1))
#define TOLOG2(x, p1, p2)               WizGlobal()->WriteLog(WizFormatString2(x, p1, p2))
#define TOLOG3(x, p1, p2, p3)           WizGlobal()->WriteLog(WizFormatString3(x, p1, p2, p3))
#define TOLOG4(x, p1, p2, p3, p4)       WizGlobal()->WriteLog(WizFormatString4(x, p1, p2, p3, p4))


#define DEBUG_TOLOG(x)                        WizGlobal()->WriteDebugLog(x)
#define DEBUG_TOLOG1(x, p1)                   WizGlobal()->WriteDebugLog(WizFormatString1(x, p1))
#define DEBUG_TOLOG2(x, p1, p2)               WizGlobal()->WriteDebugLog(WizFormatString2(x, p1, p2))
#define DEBUG_TOLOG3(x, p1, p2, p3)           WizGlobal()->WriteDebugLog(WizFormatString3(x, p1, p2, p3))
#define DEBUG_TOLOG4(x, p1, p2, p3, p4)       WizGlobal()->WriteDebugLog(WizFormatString4(x, p1, p2, p3, p4))


BOOL WizBase64Encode(const QByteArray& arrayData, QString& str);
BOOL WizBase64Decode(const QString& str, QByteArray& arrayData);

CString WizStringToBase64(const CString& strSource);
CString WizStringFromBase64(const CString& strBase64);



// skin related
QString WizGetDefaultSkinName();
void WizGetSkins(QStringList& skins);
QString WizGetSkinResourcePath(const QString& strSkinName);
QString WizGetSkinDisplayName(const QString& strSkinName, const QString& strLocale);
QString WizGetSkinResourceFileName(const QString& strSkinName, const QString& strName);
QIcon WizLoadSkinIcon(const QString& strSkinName, const QString& strIconName);


void WizHtml2Text(const QString& strHtml, QString& strText);
void WizDeleteFolder(const CString& strPath);
void WizDeleteFile(const CString& strFileName);
BOOL WizDeleteAllFilesInFolder(const CString& strPath);

BOOL WizIsValidFileNameNoPath(const CString& strFileName);
void WizMakeValidFileNameNoPath(CString& strFileName);
void WizMakeValidFileNameNoPathLimitLength(CString& strFileName, int nMaxTitleLength);
void WizMakeValidFileNameNoPathLimitFullNameLength(CString& strFileName, int nMaxFullNameLength);
CString WizMakeValidFileNameNoPathReturn(const CString& strFileName);

bool WizSaveDataToFile(const QString& strFileName, const QByteArray& arrayData);
bool WizLoadDataFromFile(const QString& strFileName, QByteArray& arrayData);


class CWizBufferAlloc
{
public:
    CWizBufferAlloc(int nInitSize = 0);
    ~CWizBufferAlloc();
private:
    BYTE* m_pBuffer;
    int m_nSize;
public:
    BYTE* GetBuffer();
    BOOL SetNewSize(int nNewSize);
};


class CWaitCursor
{
public:
    CWaitCursor();
    ~CWaitCursor();
};

#endif // WIZMISC_H
