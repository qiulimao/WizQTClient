#ifndef WIZMOBILEFILERECEIVER_H
#define WIZMOBILEFILERECEIVER_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QThread>
#include <QMutex>
#include <QTcpSocket>

struct UdpSegment
{
    QString name;
    QString type;
    long    length;
    int       totalCount;
    QByteArray data;
};

struct MobileFileData
{
    QString name;
    QString type;
    long      length;
    int         totalCount;
    QMap<int, QByteArray> data;
};

class QMutex;
class QTimer;
class QUdpSocket;
class QTcpSocket;
class QXmlStreamReader;

class CWizMobileXmlProcesser : public QThread
{
    Q_OBJECT
public:
    explicit CWizMobileXmlProcesser(QObject *parent = 0);

    void addNewSegment(QByteArray *ba);

    bool hasUnprocessedData();
    void processData();

    void waitForDone();
    void stop();

signals:
    void fileReceived(QString strFileName);

protected:
    void run();

private:
    QByteArray * peekData();
    //
    void processXML(const QByteArray& datagram);
    void processFileParam(QXmlStreamReader& xml);
    QString getElementText(QXmlStreamReader& xml);
    void addSegmentIntoDataList(const QString& strGuid, int index ,const UdpSegment& newSeg);
    bool isSegmentCompleted(const QString& strGuid);
    bool combineSegmentToFile(const QString& strGuid, QString& strFile);

private:
    QMutex m_mutex;
    QList<QByteArray *> m_segmentList;
    QMap<QString, MobileFileData> m_dataMap;
    bool m_stop;
};


class CWizMobileTcpContainer : public QThread
{
    Q_OBJECT
public:
    explicit CWizMobileTcpContainer(CWizMobileXmlProcesser *xmlProcesser, QObject *parent = 0);

    QAbstractSocket::SocketState tcpState();
    void connectToHost(const QHostAddress &address, quint16 port);

public slots:
    void readTcpPendingData();

private:
    CWizMobileXmlProcesser *m_xmlProcesser;
    QTcpSocket *m_tcpSocket;
};

class CWizMobileFileReceiver : public QThread
{
    Q_OBJECT
public:
    explicit CWizMobileFileReceiver(QObject *parent = 0);
    ~CWizMobileFileReceiver();

    void initSocket();

    void waitForDone();
    void stop();

signals:
    void fileReceived(QString strFileName);

public slots:
    void readUdpPendingData();

private:
    void getInfoFromUdpData(const QByteArray& udpData, QString& userID);

    //
    void addDataToProcesser(QByteArray *ba);
    bool isUdpSendToCurrentUser(const QString& userID);
private:
    QUdpSocket *m_udpSocket;
    CWizMobileXmlProcesser *m_xmlProcesser;
    CWizMobileTcpContainer *m_tcpContainer;
    bool m_stop;
};

#endif // WIZMOBILEFILERECEIVER_H