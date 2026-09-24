#pragma once

#include <QDialog>
#include <QObject>
#include <QStringList>

class QLabel;
class QWebChannel;
class QWebEngineView;
class QCloseEvent;

class WebRtcBridge : public QObject
{
    Q_OBJECT
public:
    explicit WebRtcBridge(QObject *parent = nullptr);

public slots:
    void sendSignal(const QString &json);
    void reportState(const QString &state);
    void reportReady();

signals:
    void localSignal(const QString &json);
    void stateChanged(const QString &state);
    void ready();
    void remoteSignal(const QString &json);
    void begin(bool initiator);
    void stop();
};

class VideoCallWindow : public QDialog
{
    Q_OBJECT
public:
    explicit VideoCallWindow(const QString &title, QWidget *parent = nullptr);
    void start(bool initiator);
    void deliverSignal(const QString &json);
    void stopFromRemote();

signals:
    void localSignal(const QString &json);
    void hangupRequested();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    WebRtcBridge *_bridge;
    QWebChannel *_channel;
    QWebEngineView *_webView;
    QLabel *_statusLabel;
    bool _pageReady = false;
    bool _startPending = false;
    bool _initiator = false;
    bool _remoteClosing = false;
    QStringList _pendingSignals;
};
