#pragma once

#include <QObject>
#include <QJsonObject>
#include "SIngleton.h"

class VideoCallWindow;

class VideoCallManager : public QObject, public Singleton<VideoCallManager>
{
    Q_OBJECT
    friend class Singleton<VideoCallManager>;
public:
    void startCall(int peerUid);

private:
    explicit VideoCallManager(QObject *parent = nullptr);
    void sendEvent(const QString &event, const QJsonObject &payload = {});
    void openWindow(bool initiator);
    void resetCall(bool closeWindow);

private slots:
    void handleEvent(QJsonObject event);

private:
    QString _callId;
    int _peerUid = 0;
    bool _busy = false;
    VideoCallWindow *_window = nullptr;
};
