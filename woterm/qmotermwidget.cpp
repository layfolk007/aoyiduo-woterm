﻿/*******************************************************************************************
*
* Copyright (C) 2023 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the GPLv3[GNU GENERAL PUBLIC LICENSE v3]
* more information follow the website: https://www.gnu.org/licenses/gpl-3.0.en.html
*
*******************************************************************************************/

#include "qmotermwidget.h"
#include "qkxtermitem.h"
#include "qwoloadingwidget.h"
#include "qwosetting.h"
#include "qwoutils.h"
#include "qwosshconf.h"
#include "qkxmessagebox.h"
#include "qkxtouchpoint.h"

#include <QApplication>
#include <QDebug>
#include <QMenu>
#include <QClipboard>
#include <QSplitter>
#include <QLabel>
#include <QTimer>
#include <QTime>
#include <QShortcut>
#include <QFileDialog>
#include <QFile>
#include <QShowEvent>
#include <QFontDatabase>


QMoTermWidget::QMoTermWidget(const QString& target, ETermType ttype, QWidget *parent)
    : QKxTermWidget(parent)
    , m_target(target)
    , m_ttype(ttype)
{
    static int idx = 0;
    setObjectName(QString("QMoTermWidget:%1").arg(idx++));
    setAttribute(Qt::WA_StyledBackground);
    setAttribute(Qt::WA_DeleteOnClose);
    initDefault();
    initCustom();


    setFocusPolicy(Qt::StrongFocus);
    setFocus();

    m_loading = new QWoLoadingWidget(QColor("#1296DB"), this);
    showTouchPoint(true, true);
    m_term->showTermName(false);

    QString val = QWoSetting::value("property/shortcut").toString();
    QVariantMap mdata = QWoUtils::qBase64ToVariant(val).toMap();
    m_term->bindShortCut(QKxTermItem::SCK_Copy, mdata.value("SCK_Copy", m_term->defaultShortCutKey(QKxTermItem::SCK_Copy)).value<QKeySequence>());
    m_term->bindShortCut(QKxTermItem::SCK_Paste, mdata.value("SCK_Paste", m_term->defaultShortCutKey(QKxTermItem::SCK_Paste)).value<QKeySequence>());
    m_term->bindShortCut(QKxTermItem::SCK_SelectAll, mdata.value("SCK_SelectAll", m_term->defaultShortCutKey(QKxTermItem::SCK_SelectAll)).value<QKeySequence>());
    m_term->bindShortCut(QKxTermItem::SCK_SelectLeft, mdata.value("SCK_SelectLeft", m_term->defaultShortCutKey(QKxTermItem::SCK_SelectLeft)).value<QKeySequence>());
    m_term->bindShortCut(QKxTermItem::SCK_SelectRight, mdata.value("SCK_SelectRight", m_term->defaultShortCutKey(QKxTermItem::SCK_SelectRight)).value<QKeySequence>());
    m_term->bindShortCut(QKxTermItem::SCK_SelectUp, mdata.value("SCK_SelectUp", m_term->defaultShortCutKey(QKxTermItem::SCK_SelectUp)).value<QKeySequence>());
    m_term->bindShortCut(QKxTermItem::SCK_SelectDown, mdata.value("SCK_SelectDown", m_term->defaultShortCutKey(QKxTermItem::SCK_SelectDown)).value<QKeySequence>());
    m_term->bindShortCut(QKxTermItem::SCK_SelectHome, mdata.value("SCK_SelectHome", m_term->defaultShortCutKey(QKxTermItem::SCK_SelectHome)).value<QKeySequence>());
    m_term->bindShortCut(QKxTermItem::SCK_SelectEnd, mdata.value("SCK_SelectEnd", m_term->defaultShortCutKey(QKxTermItem::SCK_SelectEnd)).value<QKeySequence>()); 

    // only for android.
    m_term->setBlinkAlway(true);
}

QMoTermWidget::~QMoTermWidget()
{
    if(m_loading) {
        delete m_loading;
    }
}

QString QMoTermWidget::target() const
{
    return m_target;
}

void QMoTermWidget::showLoading(bool on)
{
    if(m_loading) {
        m_loading->setVisible(on);
    }
}

void QMoTermWidget::reloadProperty()
{
    initCustom();
}

void QMoTermWidget::showEvent(QShowEvent *event)
{
    QKxTermWidget::showEvent(event);
#if defined (Q_OS_ANDROID)
#elif defined (Q_OS_IOS)
#else
    QWidget *parent = parentWidget();
    if(parent != nullptr) {
        QRect rt = parent->rect();
        setGeometry(rt);
    }
#endif
}

void QMoTermWidget::closeEvent(QCloseEvent *event)
{
    emit aboutToClose(event);
    QKxTermWidget::closeEvent(event);
}

void QMoTermWidget::resizeEvent(QResizeEvent *event)
{    
    QKxTermWidget::resizeEvent(event);
    m_loading->setGeometry(0, 0, width(), height());
    QMetaObject::invokeMethod(this, "resetTouchPointPosition", Qt::QueuedConnection);
}

void QMoTermWidget::initDefault()
{
    QVariantMap mdata = QWoSetting::ttyDefault();
    resetProperty(mdata);
}

void QMoTermWidget::initCustom()
{
    if(m_ttype == ETTLocalShell) {
        QVariantMap mdata = QWoSetting::localShell();
        resetProperty(mdata);
    }else if(m_ttype == ETTSerialPort) {
        QVariantMap mdata = QWoSetting::serialPort();
        resetProperty(mdata);
    }else{
        HostInfo hi = QWoSshConf::instance()->find(m_target);
        QVariantMap mdata = QWoUtils::qBase64ToVariant(hi.property).toMap();
        resetProperty(mdata);
    }
}

void QMoTermWidget::onResetTermSize()
{
    m_term->updateTermSize();
}

void QMoTermWidget::onCleanHistory()
{
    m_term->cleanHistory();
}

void QMoTermWidget::onOutputHistoryToFile()
{
    QString pathLast = QWoSetting::value("zmodem/lastPath").toString();
    QString file = QFileDialog::getSaveFileName(this, tr("Save history to file"), pathLast, tr("log (*.log)"));
    if(file.isEmpty()) {
        return;
    }
    QFile hit(file);
    if(!hit.open(QFile::WriteOnly)) {
        QKxMessageBox::warning(this, tr("Warning"), tr("Failed to create file."));
        return;
    }
    hit.close();
    m_term->setHistoryFile(file);
    QString path = file;
    int idx = path.lastIndexOf('/');
    if(idx > 0) {
        path = path.left(idx);
        QWoSetting::setValue("zmodem/lastPath", path);
    }
    m_historyFile = file;
}

void QMoTermWidget::onStopOutputHistoryFile()
{
    m_historyFile.clear();
    m_term->stopHistoryFile();
}

void QMoTermWidget::resetProperty(QVariantMap mdata)
{
    if(mdata.isEmpty()) {
        return;
    }
    QString schema = mdata.value("colorSchema", DEFAULT_COLOR_SCHEMA).toString();
    m_term->setColorSchema(schema);

    QString keyboard = mdata.value("keyboard", DEFAULT_KEY_LAYOUT).toString();
    m_term->setKeyLayoutByName(keyboard);

    QString codec = mdata.value("textcodec", DEFAULT_TEXT_CODEC).toString();
    m_term->setTextCodec(codec);

    QFont ft = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    QString fontName = mdata.value("fontName", ft.family()).toString();
    int fontSize = mdata.value("fontSize", ft.pointSize()).toInt();

    m_term->setTerminalFont(fontName, fontSize);

    QString cursorType = mdata.value("cursorType", "block").toString();
    if(cursorType.isEmpty() || cursorType == "block") {
        m_term->setCursorType(QKxTermItem::CT_Block);
    }else if(cursorType == "underline") {
        m_term->setCursorType(QKxTermItem::CT_Underline);
    }else {
        m_term->setCursorType(QKxTermItem::CT_IBeam);
    }
    int lines = mdata.value("historyLength", DEFAULT_HISTORY_LINE_LENGTH).toInt();
    m_term->setHistorySize(lines);
    bool dragPaste = mdata.value("dragPaste", false).toBool();
    m_term->setDragCopyAndPaste(dragPaste);
    m_rkeyPaste = mdata.value("rkeyPaste", false).toBool();
}

void QMoTermWidget::showTouchPoint(bool show, bool async)
{
    if(async == true) {
        QMetaObject::invokeMethod(this, "showTouchPoint", Qt::QueuedConnection, Q_ARG(bool, show), Q_ARG(bool, false));
        return;
    }
    if(show) {
        if(m_touchPoint == nullptr) {
            m_touchPoint = new QKxTouchPoint(m_term);
            QObject::connect(m_touchPoint, SIGNAL(clicked()), this, SIGNAL(touchPointClicked()));
        }
        QMetaObject::invokeMethod(this, "resetTouchPointPosition", Qt::QueuedConnection);
        m_touchPoint->raise();
        m_touchPoint->show();
    }else if(m_touchPoint) {
        m_touchPoint->hide();
    }
}

void QMoTermWidget::resetTouchPointPosition()
{
    if(m_touchPoint) {
        int tw = m_touchPoint->raduis() * 2;
        QSize sz = m_term->size();
        QRect rt(sz.width() - tw, (sz.height() - tw) / 2, tw, tw);
        m_touchPoint->setGeometry(rt);
    }
}
