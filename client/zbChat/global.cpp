#include "global.h"
#include <QUuid>
#include <QFile>
#include <QFileInfo>
#include <QEventLoop>
#include <QTimer>
#include <QPainter>

std::function<void(QWidget*)> repolish=[](QWidget *w){
    w->style()->unpolish(w);
    w->style()->polish(w);
    w->update();
};
QString gate_url_prefix = "";

QString generateUniqueIconName(){
    QString uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    return uuid + ".png";
}
QString generateUniqueFileName(const QString& originalName){

    QString uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    QFileInfo fileInfo(originalName);
    QString extension = fileInfo.suffix();
    return uuid + (extension.isEmpty() ? "" : "." + extension);
}
QString calculateFileHash(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return QString();

    QCryptographicHash hash(QCryptographicHash::Md5);

    // 分块计算哈希，避免大文件占用过多内存
    const qint64 chunkSize = 1024 * 1024; // 1MB
    while (!file.atEnd())
    {
        hash.addData(file.read(chunkSize));
    }
    file.close();

    return hash.result().toHex();
}
QPixmap CreateLoadingPlaceholder(int width, int height ) {
    QPixmap placeholder(width, height);
    placeholder.fill(QColor(240, 240, 240)); // 浅灰色背景

    QPainter painter(&placeholder);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制边框
    painter.setPen(QPen(QColor(200, 200, 200), 2));
    painter.drawRect(1, 1, width - 2, height - 2);

    // 绘制加载图标（简单的旋转圆圈或文字）
    QFont font;
    font.setPointSize(12);
    painter.setFont(font);
    painter.setPen(QColor(150, 150, 150));
    painter.drawText(placeholder.rect(), Qt::AlignCenter, "加载中...");

    // 可选：添加图片图标
    painter.setPen(QColor(180, 180, 180));
    QRect iconRect(width / 2 - 20, height / 2 - 40, 40, 30);
    painter.drawRect(iconRect);
    painter.drawLine(iconRect.topLeft(), iconRect.bottomRight());
    painter.drawLine(iconRect.topRight(), iconRect.bottomLeft());

    return placeholder;
}
