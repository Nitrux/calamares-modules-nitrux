/* === This file is part of Calamares - <https://calamares.io> ===
 *
 *   SPDX-FileCopyrightText: 2022 Aditya Mehra <aix.m@outlook.com>
 *   SPDX-FileCopyrightText: 2026 Uri Herrera <uri@nxos.org>
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 *   Calamares is Free Software: see the License-Identifier above.
 *
 */

#include "FlatPartitionModel.h"

#include "core/PartitionModel.h"

FlatPartitionModel::FlatPartitionModel( PartitionModel* model, QObject* parent )
    : QAbstractListModel( parent )
    , m_model( model )
{
    Q_ASSERT( m_model );

    connect( m_model, &QAbstractItemModel::modelReset, this, &FlatPartitionModel::rebuild );
    connect( m_model, &QAbstractItemModel::rowsInserted, this, &FlatPartitionModel::rebuild );
    connect( m_model, &QAbstractItemModel::rowsRemoved, this, &FlatPartitionModel::rebuild );
    connect( m_model, &QAbstractItemModel::layoutChanged, this, &FlatPartitionModel::rebuild );
    connect( m_model, &QAbstractItemModel::dataChanged, this, &FlatPartitionModel::sourceDataChanged );

    rebuild();
}

int
FlatPartitionModel::rowCount( const QModelIndex& parent ) const
{
    return parent.isValid() ? 0 : m_indexes.count();
}

QVariant
FlatPartitionModel::data( const QModelIndex& index, int role ) const
{
    if ( !index.isValid() || index.row() < 0 || index.row() >= m_indexes.count() )
    {
        return {};
    }

    const QModelIndex sourceIndex = m_indexes.at( index.row() );
    const QModelIndex nameIndex = m_model->index( sourceIndex.row(), PartitionModel::NameColumn, sourceIndex.parent() );

    switch ( role )
    {
    case Qt::DisplayRole:
    case NameRole:
        return m_model->data( nameIndex, Qt::DisplayRole );
    case FileSystemRole:
        return m_model->data(
            m_model->index( sourceIndex.row(), PartitionModel::FileSystemColumn, sourceIndex.parent() ),
            Qt::DisplayRole );
    case LabelRole:
        return m_model->data(
            m_model->index( sourceIndex.row(), PartitionModel::FileSystemLabelColumn, sourceIndex.parent() ),
            Qt::DisplayRole );
    case MountPointRole:
        return m_model->data(
            m_model->index( sourceIndex.row(), PartitionModel::MountPointColumn, sourceIndex.parent() ),
            Qt::DisplayRole );
    case SizeRole:
        return m_model->data( sourceIndex, PartitionModel::SizeRole );
    case FreeSpaceRole:
        return m_model->data( sourceIndex, PartitionModel::IsFreeSpaceRole );
    case NewPartitionRole:
        return m_model->data( sourceIndex, PartitionModel::IsPartitionNewRole );
    case PathRole:
        return m_model->data( sourceIndex, PartitionModel::PartitionPathRole );
    default:
        return {};
    }
}

QHash< int, QByteArray >
FlatPartitionModel::roleNames() const
{
    return {
        { Qt::DisplayRole, "display" },
        { NameRole, "name" },
        { FileSystemRole, "fileSystem" },
        { LabelRole, "label" },
        { MountPointRole, "mountPoint" },
        { SizeRole, "size" },
        { FreeSpaceRole, "freeSpace" },
        { NewPartitionRole, "newPartition" },
        { PathRole, "path" },
    };
}

Partition*
FlatPartitionModel::partitionAt( int row ) const
{
    if ( row < 0 || row >= m_indexes.count() )
    {
        return nullptr;
    }
    return m_model->partitionForIndex( m_indexes.at( row ) );
}

void
FlatPartitionModel::sourceDataChanged()
{
    if ( !m_indexes.isEmpty() )
    {
        Q_EMIT dataChanged( index( 0 ), index( rowCount() - 1 ) );
    }
}

void
FlatPartitionModel::rebuild()
{
    beginResetModel();
    m_indexes.clear();
    appendChildren( {} );
    endResetModel();
}

void
FlatPartitionModel::appendChildren( const QModelIndex& parent )
{
    for ( int row = 0; row < m_model->rowCount( parent ); ++row )
    {
        const QModelIndex child = m_model->index( row, PartitionModel::NameColumn, parent );
        m_indexes.append( child );
        appendChildren( child );
    }
}
