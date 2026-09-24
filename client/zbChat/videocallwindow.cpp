#include "videocallwindow.h"

#include <QCloseEvent>
#include <QLabel>
#include <QPushButton>
#include <QUrl>
#include <QVBoxLayout>
#include <QWebChannel>
#include <QWebEnginePage>
#include <QWebEngineView>

WebRtcBridge::WebRtcBridge(QObject *parent) : QObject(parent) {}

void WebRtcBridge::sendSignal(const QString &json) { emit localSignal(json); }
void WebRtcBridge::reportState(const QString &state) { emit stateChanged(state); }
void WebRtcBridge::reportReady() { emit ready(); }

VideoCallWindow::VideoCallWindow(const QString &title, QWidget *parent)
    : QDialog(parent),
      _bridge(new WebRtcBridge(this)),
      _channel(new QWebChannel(this)),
      _webView(new QWebEngineView(this)),
      _statusLabel(new QLabel(QStringLiteral("正在准备摄像头…"), this))
{
    setWindowTitle(title);
    resize(920, 680);
    setAttribute(Qt::WA_DeleteOnClose, true);

    auto *hangupButton = new QPushButton(QStringLiteral("挂断"), this);
    hangupButton->setStyleSheet("QPushButton { background:#e34d59; color:white; padding:8px 28px; border-radius:6px; }");
    auto *layout = new QVBoxLayout(this);
    layout->addWidget(_statusLabel);
    layout->addWidget(_webView, 1);
    layout->addWidget(hangupButton, 0, Qt::AlignHCenter);

    _channel->registerObject(QStringLiteral("rtcBridge"), _bridge);
    _webView->page()->setWebChannel(_channel);

    connect(_webView->page(), &QWebEnginePage::featurePermissionRequested,
            this, [this](const QUrl &origin, QWebEnginePage::Feature feature) {
        const bool media = feature == QWebEnginePage::MediaAudioCapture
                           || feature == QWebEnginePage::MediaVideoCapture
                           || feature == QWebEnginePage::MediaAudioVideoCapture;
        _webView->page()->setFeaturePermission(
            origin, feature, media ? QWebEnginePage::PermissionGrantedByUser
                                   : QWebEnginePage::PermissionDeniedByUser);
    });

    connect(_bridge, &WebRtcBridge::localSignal, this, &VideoCallWindow::localSignal);
    connect(_bridge, &WebRtcBridge::stateChanged, _statusLabel, &QLabel::setText);
    connect(_bridge, &WebRtcBridge::ready, this, [this] {
        _pageReady = true;
        for (const QString &json : _pendingSignals) {
            emit _bridge->remoteSignal(json);
        }
        _pendingSignals.clear();
        if (_startPending) {
            emit _bridge->begin(_initiator);
            _startPending = false;
        }
    });
    connect(hangupButton, &QPushButton::clicked, this, &VideoCallWindow::close);

    _webView->setUrl(QUrl(QStringLiteral("qrc:/web/webrtc.html")));
}

void VideoCallWindow::start(bool initiator)
{
    _initiator = initiator;
    if (_pageReady) {
        emit _bridge->begin(initiator);
    } else {
        _startPending = true;
    }
}

void VideoCallWindow::deliverSignal(const QString &json)
{
    if (_pageReady) emit _bridge->remoteSignal(json);
    else _pendingSignals.append(json);
}

void VideoCallWindow::stopFromRemote()
{
    _remoteClosing = true;
    emit _bridge->stop();
    close();
}

void VideoCallWindow::closeEvent(QCloseEvent *event)
{
    emit _bridge->stop();
    if (!_remoteClosing) {
        emit hangupRequested();
    }
    QDialog::closeEvent(event);
}
