#include "videocallmanager.h"

#include "global.h"
#include "tcpmgr.h"
#include "usermgr.h"
#include "videocallwindow.h"
#include <QJsonDocument>
#include <QMessageBox>
#include <QTimer>
#include <QUuid>

VideoCallManager::VideoCallManager(QObject *parent) : QObject(parent)
{
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_video_call_event,
            this, &VideoCallManager::handleEvent, Qt::QueuedConnection);
}

void VideoCallManager::startCall(int peerUid)
{
    if (_busy || peerUid <= 0) {
        QMessageBox::information(nullptr, QStringLiteral("视频通话"), QStringLiteral("当前已有通话正在进行"));
        return;
    }
    _busy = true;
    _peerUid = peerUid;
    _callId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    sendEvent(QStringLiteral("invite"));
    const QString pendingCallId = _callId;
    QTimer::singleShot(30000, this, [this, pendingCallId] {
        if (_busy && _callId == pendingCallId && !_window) {
            sendEvent(QStringLiteral("hangup"));
            resetCall(false);
            QMessageBox::information(nullptr, QStringLiteral("视频通话"), QStringLiteral("对方暂未接听"));
        }
    });
}

void VideoCallManager::sendEvent(const QString &event, const QJsonObject &payload)
{
    QJsonObject object;
    object["event"] = event;
    object["call_id"] = _callId;
    object["fromuid"] = UserMgr::GetInstance()->GetUId();
    object["touid"] = _peerUid;
    if (!payload.isEmpty()) object["payload"] = payload;
    emit TcpMgr::GetInstance()->sig_send_data(
        ID_VIDEO_CALL_EVENT_REQ,
        QJsonDocument(object).toJson(QJsonDocument::Compact));
}

void VideoCallManager::openWindow(bool initiator)
{
    if (_window) return;
    _window = new VideoCallWindow(QStringLiteral("与用户 %1 视频通话").arg(_peerUid));
    connect(_window, &VideoCallWindow::localSignal, this, [this](const QString &json) {
        const QJsonDocument document = QJsonDocument::fromJson(json.toUtf8());
        if (document.isObject()) sendEvent(QStringLiteral("signal"), document.object());
    });
    connect(_window, &VideoCallWindow::hangupRequested, this, [this] {
        if (_busy) sendEvent(QStringLiteral("hangup"));
        resetCall(false);
    });
    connect(_window, &QObject::destroyed, this, [this] { _window = nullptr; });
    _window->show();
    _window->start(initiator);
}

void VideoCallManager::handleEvent(QJsonObject object)
{
    const QString event = object["event"].toString();
    const QString callId = object["call_id"].toString();
    const int fromUid = object["fromuid"].toInt();

    if (event == QStringLiteral("invite")) {
        if (_busy) {
            const QString oldId = _callId;
            const int oldPeer = _peerUid;
            _callId = callId;
            _peerUid = fromUid;
            sendEvent(QStringLiteral("busy"));
            _callId = oldId;
            _peerUid = oldPeer;
            return;
        }
        _busy = true;
        _callId = callId;
        _peerUid = fromUid;
        const auto choice = QMessageBox::question(nullptr, QStringLiteral("视频来电"),
                                                   QStringLiteral("用户 %1 邀请你视频通话，是否接听？").arg(fromUid));
        if (choice == QMessageBox::Yes) {
            sendEvent(QStringLiteral("accept"));
            openWindow(false);
        } else {
            sendEvent(QStringLiteral("reject"));
            resetCall(false);
        }
        return;
    }

    if (!_busy || callId != _callId || fromUid != _peerUid) return;
    if (event == QStringLiteral("accept")) {
        openWindow(true);
    } else if (event == QStringLiteral("signal") && _window) {
        _window->deliverSignal(QString::fromUtf8(
            QJsonDocument(object["payload"].toObject()).toJson(QJsonDocument::Compact)));
    } else if (event == QStringLiteral("reject") || event == QStringLiteral("busy")) {
        QMessageBox::information(nullptr, QStringLiteral("视频通话"),
                                 event == QStringLiteral("busy") ? QStringLiteral("对方正在通话中")
                                                                  : QStringLiteral("对方拒绝了通话"));
        resetCall(true);
    } else if (event == QStringLiteral("hangup")) {
        resetCall(true);
    }
}

void VideoCallManager::resetCall(bool closeWindow)
{
    VideoCallWindow *window = _window;
    _window = nullptr;
    _busy = false;
    _callId.clear();
    _peerUid = 0;
    if (closeWindow && window) window->stopFromRemote();
}
