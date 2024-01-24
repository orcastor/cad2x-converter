/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/


#include "qtextimagehandler_p.h"

#include <qtextformat.h>
#include <qpainter.h>
#include <qdebug.h>
#include <qfile.h>
#include <private/qtextengine_p.h>
#include <qpalette.h>
#include <qthread.h>
#include <QtCore/qglobal.h>
#include <QtCore/qmath.h>

QT_BEGIN_NAMESPACE


/*!
    \internal
    \since 5.6
    Attempts to find a suitable @Nx file for the given \a targetDevicePixelRatio
    Returns the the \a baseFileName if no such file was found.

    Given base foo.png and a target dpr of 2.5, this function will look for
    foo@3x.png, then foo@2x, then fall back to foo.png if not found.

    \a sourceDevicePixelRatio will be set to the value of N if the argument is
    a non-null pointer
*/
QString qt_findAtNxFile(const QString &baseFileName, qreal targetDevicePixelRatio,
                        qreal *sourceDevicePixelRatio)
{
    if (targetDevicePixelRatio <= 1.0)
        return baseFileName;

    static bool disableNxImageLoading = !qEnvironmentVariableIsEmpty("QT_HIGHDPI_DISABLE_2X_IMAGE_LOADING");
    if (disableNxImageLoading)
        return baseFileName;

    int dotIndex = baseFileName.lastIndexOf(QLatin1Char('.'));
    if (dotIndex == -1) { /* no dot */
        dotIndex = baseFileName.size(); /* append */
    } else if (dotIndex >= 2 && baseFileName[dotIndex - 1] == QLatin1Char('9')
        && baseFileName[dotIndex - 2] == QLatin1Char('.')) {
        // If the file has a .9.* (9-patch image) extension, we must ensure that the @nx goes before it.
        dotIndex -= 2;
    }

    QString atNxfileName = baseFileName;
    atNxfileName.insert(dotIndex, QLatin1String("@2x"));
    // Check for @Nx, ..., @3x, @2x file versions,
    for (int n = qMin(qCeil(targetDevicePixelRatio), 9); n > 1; --n) {
        atNxfileName[dotIndex + 1] = QLatin1Char('0' + n);
        if (QFile::exists(atNxfileName)) {
            if (sourceDevicePixelRatio)
                *sourceDevicePixelRatio = n;
            return atNxfileName;
        }
    }

    return baseFileName;
}

static QString resolveFileName(QString fileName, QUrl *url, qreal targetDevicePixelRatio,
                               qreal *sourceDevicePixelRatio)
{
    // We might use the fileName for loading if url loading fails
    // try to make sure it is a valid file path.
    // Also, QFile{Info}::exists works only on filepaths (not urls)

    if (url->isValid()) {
      if (url->scheme() == QLatin1String("qrc")) {
        fileName = fileName.right(fileName.length() - 3);
      }
      else if (url->scheme() == QLatin1String("file")) {
        fileName = url->toLocalFile();
      }
    }

    if (targetDevicePixelRatio <= 1.0)
        return fileName;

    // try to find a Nx version
    return qt_findAtNxFile(fileName, targetDevicePixelRatio, sourceDevicePixelRatio);
}

static QImage getImage(QTextDocument *doc, const QTextImageFormat &format, const qreal devicePixelRatio = 1.0)
{
    QImage image;

    QString name = format.name();
    if (name.startsWith(QLatin1String(":/"))) // auto-detect resources
        name.prepend(QLatin1String("qrc"));
    QUrl url = QUrl(name);
    qreal sourcePixelRatio = 1.0;
    name = resolveFileName(name, &url, devicePixelRatio, &sourcePixelRatio);
    const QVariant data = doc->resource(QTextDocument::ImageResource, url);
    if (data.type() == QVariant::Image) {
        image = qvariant_cast<QImage>(data);
    } else if (data.type() == QVariant::ByteArray) {
        image.loadFromData(data.toByteArray());
    }

    if (image.isNull()) {
#if 0
        QString context;
        // ### Qt5
        QTextBrowser *browser = qobject_cast<QTextBrowser *>(doc->parent());
        if (browser)
            context = browser->source().toString();
#endif
        // try direct loading

        if (name.isEmpty() || !image.load(name))
            return QImage(QLatin1String(":/qt-project.org/styles/commonstyle/images/file-16.png"));

        doc->addResource(QTextDocument::ImageResource, url, image);
    }

    if (sourcePixelRatio != 1.0)
        image.setDevicePixelRatio(sourcePixelRatio);

    return image;
}

static QSize getImageSize(QTextDocument *doc, const QTextImageFormat &format)
{
    QImage image;

    const bool hasWidth = format.hasProperty(QTextFormat::ImageWidth);
    const int width = qRound(format.width());
    const bool hasHeight = format.hasProperty(QTextFormat::ImageHeight);
    const int height = qRound(format.height());

    QSize size(width, height);
    if (!hasWidth || !hasHeight) {
        image = getImage(doc, format);
        if (!hasWidth)
            size.setWidth(image.width() / image.devicePixelRatio());
        if (!hasHeight)
            size.setHeight(image.height() / image.devicePixelRatio());
    }

    qreal scale = 1.0;
    QPaintDevice *pdev = doc->documentLayout()->paintDevice();
    if (pdev) {
        if (image.isNull())
            image = getImage(doc, format);
        if (!image.isNull())
            scale = qreal(pdev->logicalDpiY()) / qreal(qt_defaultDpi());
    }
    size *= scale;

    return size;
}

QTextImageHandler::QTextImageHandler(QObject *parent)
    : QObject(parent)
{
}

QSizeF QTextImageHandler::intrinsicSize(QTextDocument *doc, int posInDocument, const QTextFormat &format)
{
    Q_UNUSED(posInDocument)
    const QTextImageFormat imageFormat = format.toImageFormat();
    return getImageSize(doc, imageFormat);
}

QImage QTextImageHandler::image(QTextDocument *doc, const QTextImageFormat &imageFormat)
{
    Q_ASSERT(doc != 0);

    return getImage(doc, imageFormat);
}

void QTextImageHandler::drawObject(QPainter *p, const QRectF &rect, QTextDocument *doc, int posInDocument, const QTextFormat &format)
{
    Q_UNUSED(posInDocument)
    const QTextImageFormat imageFormat = format.toImageFormat();

    const QImage image = getImage(doc, imageFormat, p->device()->devicePixelRatioF());
    p->drawImage(rect, image, image.rect());
}

QT_END_NAMESPACE
