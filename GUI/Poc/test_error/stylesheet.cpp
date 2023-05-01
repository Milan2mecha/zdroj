#include "stylesheet.h"

stylesheet::stylesheet(QObject *parent)
    : QAbstractItemModel(parent)
{
}

QVariant stylesheet::headerData(int section, Qt::Orientation orientation, int role) const
{
    // FIXME: Implement me!
}

QModelIndex stylesheet::index(int row, int column, const QModelIndex &parent) const
{
    // FIXME: Implement me!
}

QModelIndex stylesheet::parent(const QModelIndex &index) const
{
    // FIXME: Implement me!
}

int stylesheet::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return 0;

    // FIXME: Implement me!
}

int stylesheet::columnCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return 0;

    // FIXME: Implement me!
}

QVariant stylesheet::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    // FIXME: Implement me!
    return QVariant();
}
