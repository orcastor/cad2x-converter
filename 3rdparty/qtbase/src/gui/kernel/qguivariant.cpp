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

// Gui types
#include "qbrush.h"
#include "qcolor.h"
#include "qfont.h"
#include "qimage.h"
#include "qtransform.h"
#include "qmatrix.h"
#include "qpalette.h"
#include "qpen.h"
#include "qpolygon.h"
#include "qregion.h"
#include "qtextformat.h"
#include "qmatrix4x4.h"
#include "qvector2d.h"
#include "qvector3d.h"
#include "qvector4d.h"
#include "qquaternion.h"

// Core types
#include "qvariant.h"
#include "qbitarray.h"
#include "qbytearray.h"
#include "qdatastream.h"
#include "qdebug.h"
#include "qmap.h"
#include "qdatetime.h"
#include "qeasingcurve.h"
#include "qlist.h"
#include "qstring.h"
#include "qstringlist.h"
#include "qurl.h"
#include "qlocale.h"
#include "quuid.h"

#ifndef QT_NO_GEOM_VARIANT
#include "qsize.h"
#include "qpoint.h"
#include "qrect.h"
#include "qline.h"
#endif

#include <float.h>

#include "private/qvariant_p.h"
#include <private/qmetatype_p.h>

QT_BEGIN_NAMESPACE

Q_CORE_EXPORT const QVariant::Handler *qcoreVariantHandler();

namespace {
struct GuiTypesFilter {
    template<typename T>
    struct Acceptor {
        static const bool IsAccepted = QModulesPrivate::QTypeModuleInfo<T>::IsGui && QtMetaTypePrivate::TypeDefinition<T>::IsAvailable;
    };
};

static void construct(QVariant::Private *x, const void *copy)
{
    const int type = x->type;
    QVariantConstructor<GuiTypesFilter> constructor(x, copy);
    QMetaTypeSwitcher::switcher<void>(constructor, type, 0);
}

static void clear(QVariant::Private *d)
{
    QVariantDestructor<GuiTypesFilter> destructor(d);
    QMetaTypeSwitcher::switcher<void>(destructor, d->type, 0);
}

// This class is a hack that customizes access to QPolygon and QPolygonF
template<class Filter>
class QGuiVariantIsNull : public QVariantIsNull<Filter> {
    typedef QVariantIsNull<Filter> Base;
public:
    QGuiVariantIsNull(const QVariant::Private *d)
        : QVariantIsNull<Filter>(d)
    {}
    template<typename T>
    bool delegate(const T *p) { return Base::delegate(p); }
    bool delegate(const QPolygon*) { return v_cast<QPolygon>(Base::m_d)->isEmpty(); }
    bool delegate(const QPolygonF*) { return v_cast<QPolygonF>(Base::m_d)->isEmpty(); }
    bool delegate(const void *p) { return Base::delegate(p); }
};
static bool isNull(const QVariant::Private *d)
{
    QGuiVariantIsNull<GuiTypesFilter> isNull(d);
    return QMetaTypeSwitcher::switcher<bool>(isNull, d->type, 0);
}

// This class is a hack that customizes access to QPixmap, QBitmap, QCursor and QIcon
template<class Filter>
class QGuiVariantComparator : public QVariantComparator<Filter> {
    typedef QVariantComparator<Filter> Base;
public:
    QGuiVariantComparator(const QVariant::Private *a, const QVariant::Private *b)
        : QVariantComparator<Filter>(a, b)
    {}
    template<typename T>
    bool delegate(const T *p)
    {
        return Base::delegate(p);
    }
    bool delegate(const void *p) { return Base::delegate(p); }
};

static bool compare(const QVariant::Private *a, const QVariant::Private *b)
{
    QGuiVariantComparator<GuiTypesFilter> comparator(a, b);
    return QMetaTypeSwitcher::switcher<bool>(comparator, a->type, 0);
}

static bool convert(const QVariant::Private *d, int t,
                 void *result, bool *ok)
{
    switch (t) {
    case QVariant::ByteArray:
        if (d->type == QVariant::Color) {
            const QColor *c = v_cast<QColor>(d);
            *static_cast<QByteArray *>(result) = c->name(c->alpha() != 255 ? QColor::HexArgb : QColor::HexRgb).toLatin1();
            return true;
        }
        break;
    case QVariant::String: {
        QString *str = static_cast<QString *>(result);
        switch (d->type) {
        case QVariant::Font:
            *str = v_cast<QFont>(d)->toString();
            return true;
        case QVariant::Color: {
            const QColor *c = v_cast<QColor>(d);
            *str = c->name(c->alpha() != 255 ? QColor::HexArgb : QColor::HexRgb);
            return true;
        }
        default:
            break;
        }
        break;
    }
    case QVariant::Font:
        if (d->type == QVariant::String) {
            QFont *f = static_cast<QFont *>(result);
            f->fromString(*v_cast<QString>(d));
            return true;
        }
        break;
    case QVariant::Color:
        if (d->type == QVariant::String) {
            static_cast<QColor *>(result)->setNamedColor(*v_cast<QString>(d));
            return static_cast<QColor *>(result)->isValid();
        } else if (d->type == QVariant::ByteArray) {
            static_cast<QColor *>(result)->setNamedColor(QLatin1String(*v_cast<QByteArray>(d)));
            return true;
        } else if (d->type == QVariant::Brush) {
            if (v_cast<QBrush>(d)->style() == Qt::SolidPattern) {
                *static_cast<QColor *>(result) = v_cast<QBrush>(d)->color();
                return true;
            }
        }
        break;
    case QVariant::Brush:
        if (d->type == QVariant::Color) {
            *static_cast<QBrush *>(result) = QBrush(*v_cast<QColor>(d));
            return true;
        }
        break;
    default:
        break;
    }
    return qcoreVariantHandler()->convert(d, t, result, ok);
}

#if !defined(QT_NO_DEBUG_STREAM)
static void streamDebug(QDebug dbg, const QVariant &v)
{
    QVariant::Private *d = const_cast<QVariant::Private *>(&v.data_ptr());
    QVariantDebugStream<GuiTypesFilter> stream(dbg, d);
    QMetaTypeSwitcher::switcher<void>(stream, d->type, 0);
}
#endif

const QVariant::Handler qt_gui_variant_handler = {
    construct,
    clear,
    isNull,
#ifndef QT_NO_DATASTREAM
    0,
    0,
#endif
    compare,
    convert,
    0,
#if !defined(QT_NO_DEBUG_STREAM)
    streamDebug
#else
    0
#endif
};

#define QT_IMPL_METATYPEINTERFACE_GUI_TYPES(MetaTypeName, MetaTypeId, RealName) \
    QT_METATYPE_INTERFACE_INIT(RealName),

static const QMetaTypeInterface qVariantGuiHelper[] = {
    QT_FOR_EACH_STATIC_GUI_CLASS(QT_IMPL_METATYPEINTERFACE_GUI_TYPES)
};

#undef QT_IMPL_METATYPEINTERFACE_GUI_TYPES
} // namespace used to hide QVariant handler

extern Q_CORE_EXPORT const QMetaTypeInterface *qMetaTypeGuiHelper;

void qRegisterGuiVariant()
{
    QVariantPrivate::registerHandler(QModulesPrivate::Gui, &qt_gui_variant_handler);
    qMetaTypeGuiHelper = qVariantGuiHelper;
}
Q_CONSTRUCTOR_FUNCTION(qRegisterGuiVariant)

QT_END_NAMESPACE
