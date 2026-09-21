/* === This file is part of Calamares - <https://calamares.io> ===
 *
 *   SPDX-FileCopyrightText: 2022 Aditya Mehra <aix.m@outlook.com>
 *   SPDX-FileCopyrightText: 2026 Uri Herrera <uri@nxos.org>
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 *   Calamares is Free Software: see the License-Identifier above.
 *
 */

#ifndef NITRUX_FLATPARTITIONMODEL_H
#define NITRUX_FLATPARTITIONMODEL_H

#include <QAbstractListModel>
#include <QModelIndex>
#include <QVector>

class Partition;
class PartitionModel;

class FlatPartitionModel final : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Role
    {
        NameRole = Qt::UserRole + 1,
        FileSystemRole,
        LabelRole,
        MountPointRole,
        SizeRole,
        FreeSpaceRole,
        NewPartitionRole,
        PathRole,
    };
    Q_ENUM( Role )

    explicit FlatPartitionModel( PartitionModel* model, QObject* parent = nullptr );

    int rowCount( const QModelIndex& parent = QModelIndex() ) const override;
    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const override;
    QHash< int, QByteArray > roleNames() const override;

    Partition* partitionAt( int row ) const;

private Q_SLOTS:
    void sourceDataChanged();

private:
    void rebuild();
    void appendChildren( const QModelIndex& parent );

    PartitionModel* m_model;
    QVector< QModelIndex > m_indexes;
};

#endif
