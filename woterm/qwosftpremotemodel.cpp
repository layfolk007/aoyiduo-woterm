﻿/*******************************************************************************************
*
* Copyright (C) 2022 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the GPLv3[GNU GENERAL PUBLIC LICENSE v3]
* more information follow the website: https://www.gnu.org/licenses/gpl-3.0.en.html
*
*******************************************************************************************/

#include "qwosftpremotemodel.h"

#include <QPair>
#include <QFontMetrics>
#include <QVector>
#include <QDebug>
#include <QIcon>
#include <QFont>
#include <QGuiApplication>

QWoSftpRemoteModel::QWoSftpRemoteModel(QObject *parent)
    : QAbstractListModel (parent)
{
    m_font = QGuiApplication::font();
    m_dirIcon = QIcon(QPixmap("../private/skins/black/folder2.png").scaled(24, 24, Qt::KeepAspectRatio ,Qt::SmoothTransformation));
    m_fileIcon = QIcon(QPixmap("../private/skins/black/file.png").scaled(24, 24, Qt::KeepAspectRatio ,Qt::SmoothTransformation));
    m_linkIcon = QIcon(QPixmap("../private/skins/black/link.png").scaled(24, 24, Qt::KeepAspectRatio ,Qt::SmoothTransformation));
}

QWoSftpRemoteModel::~QWoSftpRemoteModel()
{

}

QString QWoSftpRemoteModel::path() const
{
    return m_path;
}

bool QWoSftpRemoteModel::exist(const QString &fileName)
{
    auto it = std::find_if(m_fileInfos.begin(), m_fileInfos.end(), [=](const FileInfo& fi) {
        return fi.name == fileName;
    });
    return it != m_fileInfos.end();
}

int QWoSftpRemoteModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()){
        return 0;
    }    
    return m_fileInfos.count();
}

QVariant QWoSftpRemoteModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role == Qt::DisplayRole) {
        switch (section) {
        case 0:
            return tr("Name");
        case 1:
            return tr("Permission");
        case 2:
            return tr("Owner");
        case 3:
            return tr("Group");
        case 4:
            return tr("Size");
        case 5:
            return tr("Date");
        }
    }
    return QAbstractListModel::headerData(section, orientation, role);
}

QVariant QWoSftpRemoteModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_fileInfos.size()) {
        return QVariant();
    }
    if (!index.isValid()){
        return QVariant();
    }
    if(role == Qt::FontRole) {
        return m_font;
    }
    const FileInfo& fi = m_fileInfos.at(index.row());
    int column = index.column();

    if(role == ROLE_INDEX) {
        return index.row();
    }

    if(role == ROLE_FILEINFO) {
        QVariant v;
        v.setValue(fi);
        return v;
    }
    if(role == ROLE_REFILTER) {
        QVariant v;
        v.setValue(QString("%1").arg(fi.name));
        return v;
    }
    if(role == ROLE_FRIENDLY_NAME) {
        QVariant v;
        v.setValue(fi.name);
        return v;
    }
    if(role == Qt::SizeHintRole) {
        QFontMetrics fm(m_font);
        if(column == 0) {
            int w = fm.width(fi.name) + 50;
            return QSize(w, 24);
        }
        if(column == 1) {
            int w = fm.width(fi.permission) + 10;
            return QSize(w, 24);
        }
        if(column == 2) {
            int w = fm.width(fi.owner) + 10;
            return QSize(w, 24);
        }
        if(column == 3) {
            int w = fm.width(fi.group) + 10;
            return QSize(w, 24);
        }
        if(column == 4) {
            int w = fm.width(QString::number(fi.size)) + 10;
            return QSize(w, 24);
        }
        if(column == 5) {
            int w = fm.width(fi.date) + 10;
            return QSize(w, 24);
        }
    }

    if(column == 0) {
        if(role == Qt::DecorationRole) {
            if(fi.type == "d") {
                return m_dirIcon;
            }else if(fi.type == "l") {
                return m_linkIcon;
            }
            return m_fileIcon;
        }else if(role == Qt::DisplayRole) {
            return fi.name;
        }
        return QVariant();
    }
    if(role != Qt::DisplayRole) {
        return QVariant();
    }
    if(column == 1) {
        return fi.permission;
    }
    if(column == 2) {
        return fi.owner;
    }
    if(column == 3) {
        return fi.group;
    }
    if(column == 4) {
        return fi.size;
    }
    if(column == 5) {
        return fi.date;
    }
    return QVariant();
}



bool QWoSftpRemoteModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    return false;
}

Qt::ItemFlags QWoSftpRemoteModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled;
}

bool QWoSftpRemoteModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if (count < 1 || row < 0 || row > rowCount(parent)){
        return false;
    }

    beginInsertRows(QModelIndex(), row, row + count - 1);

//    for (int r = 0; r < count; ++r){
//        m_hosts.insert(row, HostInfo());
//    }

    endInsertRows();

    return QAbstractListModel::insertRows(row, count, parent);
}

bool QWoSftpRemoteModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (count <= 0 || row < 0 || (row + count) > rowCount(parent)){
        return false;
    }

    beginRemoveRows(QModelIndex(), row, row + count - 1);

//    const auto it = m_hosts.begin() + row;
//    m_hosts.erase(it, it + count);

    endRemoveRows();

    return true;
}

static bool ascendingLessThan(const QPair<QString, int> &s1, const QPair<QString, int> &s2)
{
    return s1.first < s2.first;
}

static bool decendingLessThan(const QPair<QString, int> &s1, const QPair<QString, int> &s2)
{
    return s1.first > s2.first;
}

void QWoSftpRemoteModel::sort(int column, Qt::SortOrder order)
{
#if 1
    return QAbstractListModel::sort(column, order);
#else
    emit layoutAboutToBeChanged(QList<QPersistentModelIndex>(), VerticalSortHint);

     QVector<QPair<QString, HostInfo> > list;
     const int lstCount = m_hosts.count();
     list.reserve(lstCount);
     for (int i = 0; i < lstCount; ++i){
         list.append(QPair<QString, HostInfo>(m_hosts.at(i), i));
     }

     if (order == Qt::AscendingOrder)
         std::sort(list.begin(), list.end(), ascendingLessThan);
     else
         std::sort(list.begin(), list.end(), decendingLessThan);

     m_hosts.clear();
     QVector<int> forwarding(lstCount);
     for (int i = 0; i < lstCount; ++i) {
         m_hosts.append(list.at(i).first);
         forwarding[list.at(i).second] = i;
     }

     QModelIndexList oldList = persistentIndexList();
     QModelIndexList newList;
     const int numOldIndexes = oldList.count();
     newList.reserve(numOldIndexes);
     for (int i = 0; i < numOldIndexes; ++i)
         newList.append(index(forwarding.at(oldList.at(i).row()), 0));
     changePersistentIndexList(oldList, newList);

     emit layoutChanged(QList<QPersistentModelIndex>(), VerticalSortHint);
#endif
}

int QWoSftpRemoteModel::columnCount(const QModelIndex &parent) const
{
    return 6;
}

Qt::DropActions QWoSftpRemoteModel::supportedDropActions() const
{
    return QAbstractListModel::supportedDropActions();
}

static bool lessThan(const FileInfo &s1, const FileInfo &s2)
{
    static QString type = "dl-";
    int t1 = type.indexOf(s1.type);
    int t2 = type.indexOf(s2.type);
    if(t1 < t2) {
        return true;
    }
    if(t1 > t2) {
        return false;
    }
    return s1.name < s2.name;
}

void QWoSftpRemoteModel::onDirOpen(const QString &path, const QList<QVariant> &v, const QVariantMap& userData)
{
    beginResetModel();
    m_path = path;
    m_fileInfos.clear();
    for(int i = 0; i < v.length(); i++) {
        QMap<QString, QVariant> mdata = v.at(i).toMap();
        FileInfo fi;
        fi.longName = mdata.value("longName").toString();
        fi.label = mdata.value("label").toString();
        fi.name = mdata.value("name").toString();
        fi.type = mdata.value("type").toString();
        fi.owner = mdata.value("owner").toString();
        fi.group = mdata.value("group").toString();
        fi.size = mdata.value("size").toLongLong();
        fi.date = mdata.value("date").toString();
        fi.permission = mdata.value("permission").toString();
        m_fileInfos.append(fi);
    }
    std::sort(m_fileInfos.begin(), m_fileInfos.end(), lessThan);
    endResetModel();

    emit pathChanged(m_path);
}
