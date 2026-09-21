/* === This file is part of Calamares - <https://calamares.io> ===
 *
 *   SPDX-FileCopyrightText: 2026 Uri Herrera <uri@nxos.org>
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 *   Calamares is Free Software: see the License-Identifier above.
 *
 */

#include "ChoicePageQml.h"

#include "Config.h"
#include "FlatPartitionModel.h"
#include "core/DeviceModel.h"
#include "core/PartUtils.h"
#include "core/PartitionActions.h"
#include "core/PartitionCoreModule.h"
#include "core/PartitionModel.h"

#include "GlobalStorage.h"
#include "JobQueue.h"
#include "partition/PartitionQuery.h"
#include "utils/Logger.h"
#include "utils/Units.h"

#include <kpmcore/core/device.h>
#include <kpmcore/core/partition.h>

#include <cmath>
#include <limits>

ChoicePageQml::ChoicePageQml( Config* config, QObject* parent )
    : QObject( parent )
    , m_config( config )
{
}

void
ChoicePageQml::initialize( PartitionCoreModule* core )
{
    if ( m_ready )
    {
        return;
    }

    m_core = core;
    m_ready = true;

    if ( m_core && m_core->deviceModel() && m_core->deviceModel()->rowCount() > 0 )
    {
        m_deviceIndex = 0;
        refreshPartitionModel();
    }

    Q_EMIT deviceModelChanged();
    Q_EMIT currentDeviceIndexChanged();
    Q_EMIT readyChanged();
    updateNextStatus();
}

QAbstractItemModel*
ChoicePageQml::deviceModel() const
{
    return m_core ? m_core->deviceModel() : nullptr;
}

QAbstractItemModel*
ChoicePageQml::partitionModel() const
{
    return m_partitionModel;
}

int
ChoicePageQml::currentDeviceIndex() const
{
    return m_deviceIndex;
}

void
ChoicePageQml::setCurrentDeviceIndex( int index )
{
    if ( !m_core || !m_core->deviceModel() )
    {
        return;
    }

    if ( index < 0 || index >= m_core->deviceModel()->rowCount() )
    {
        return;
    }

    if ( index == m_deviceIndex )
    {
        return;
    }

    if ( m_core->isDirty() )
    {
        m_core->revertAllDevices();
    }

    m_deviceIndex = index;
    m_config->setInstallChoice( Config::InstallChoice::NoChoice );
    refreshPartitionModel();

    Q_EMIT currentDeviceIndexChanged();
    Q_EMIT installChoiceChanged();
    updateNextStatus();
}

int
ChoicePageQml::installChoice() const
{
    return m_config ? static_cast< int >( m_config->installChoice() ) : NoChoice;
}

bool
ChoicePageQml::ready() const
{
    return m_ready;
}

bool
ChoicePageQml::eraseEnabled() const
{
    return m_ready && selectedDevice();
}

bool
ChoicePageQml::replaceEnabled() const
{
    return m_ready && m_partitionModel && m_partitionModel->rowCount() > 0;
}

bool
ChoicePageQml::alongsideEnabled() const
{
    return false;
}

bool
ChoicePageQml::manualEnabled() const
{
    return false;
}

QString
ChoicePageQml::currentDeviceName() const
{
    Device* device = selectedDevice();
    return device ? device->name() : QString();
}

bool
ChoicePageQml::encryptionEnabled() const
{
    return m_encryptionEnabled;
}

bool
ChoicePageQml::encryptionChecked() const
{
    return m_encryptionChecked;
}

void
ChoicePageQml::setEncryptionChecked( bool checked )
{
    const bool oldValid = encryptionValid();
    const bool effectiveChecked = m_encryptionEnabled && checked;
    if ( effectiveChecked == m_encryptionChecked )
    {
        return;
    }

    m_encryptionChecked = effectiveChecked;
    Q_EMIT encryptionCheckedChanged();

    if ( !m_encryptionChecked )
    {
        if ( !m_encryptPassphrase.isEmpty() )
        {
            m_encryptPassphrase.clear();
            Q_EMIT encryptionPassphraseChanged();
        }
        if ( !m_encryptPassphraseConfirmation.isEmpty() )
        {
            m_encryptPassphraseConfirmation.clear();
            Q_EMIT encryptionPassphraseConfirmationChanged();
        }
    }

    if ( oldValid != encryptionValid() )
    {
        Q_EMIT encryptionValidityChanged();
    }
    reapplyChoice();
    updateNextStatus();
}

QString
ChoicePageQml::encryptionPassphrase() const
{
    return m_encryptPassphrase;
}

void
ChoicePageQml::setEncryptionPassphrase( const QString& passphrase )
{
    if ( passphrase == m_encryptPassphrase )
    {
        return;
    }

    const bool oldValid = encryptionValid();
    m_encryptPassphrase = passphrase;
    Q_EMIT encryptionPassphraseChanged();
    if ( oldValid != encryptionValid() )
    {
        Q_EMIT encryptionValidityChanged();
    }
    reapplyChoice();
    updateNextStatus();
}

QString
ChoicePageQml::encryptionPassphraseConfirmation() const
{
    return m_encryptPassphraseConfirmation;
}

void
ChoicePageQml::setEncryptionPassphraseConfirmation( const QString& passphrase )
{
    if ( passphrase == m_encryptPassphraseConfirmation )
    {
        return;
    }

    const bool oldValid = encryptionValid();
    m_encryptPassphraseConfirmation = passphrase;
    Q_EMIT encryptionPassphraseConfirmationChanged();
    if ( oldValid != encryptionValid() )
    {
        Q_EMIT encryptionValidityChanged();
    }
    reapplyChoice();
    updateNextStatus();
}

bool
ChoicePageQml::encryptionValid() const
{
    return !m_encryptionChecked
        || ( !m_encryptPassphrase.isEmpty() && m_encryptPassphrase == m_encryptPassphraseConfirmation );
}

void
ChoicePageQml::setEncryptionEnabled( bool enabled )
{
    if ( enabled == m_encryptionEnabled )
    {
        return;
    }

    m_encryptionEnabled = enabled;
    m_encryptionChecked = m_encryptionEnabled && m_config && m_config->preCheckEncryption();
    if ( !m_encryptionEnabled )
    {
        m_encryptionChecked = false;
        m_encryptPassphrase.clear();
        m_encryptPassphraseConfirmation.clear();
    }
    Q_EMIT encryptionEnabledChanged();
    Q_EMIT encryptionCheckedChanged();
    Q_EMIT encryptionPassphraseChanged();
    Q_EMIT encryptionPassphraseConfirmationChanged();
    Q_EMIT encryptionValidityChanged();
    updateNextStatus();
}

bool
ChoicePageQml::isNextEnabled() const
{
    if ( !m_ready || !selectedDevice() )
    {
        return false;
    }

    return ( m_config->installChoice() == Config::InstallChoice::Erase
             || m_config->installChoice() == Config::InstallChoice::Replace )
        && encryptionValid();
}

bool
ChoicePageQml::isAtEnd() const
{
    return isNextEnabled();
}

bool
ChoicePageQml::chooseErase()
{
    if ( !m_core || !selectedDevice() || !encryptionValid() )
    {
        return false;
    }

    if ( !applyErase() )
    {
        return false;
    }

    m_config->setInstallChoice( Config::InstallChoice::Erase );
    Q_EMIT installChoiceChanged();
    Q_EMIT choiceApplied();
    updateNextStatus();
    return true;
}

bool
ChoicePageQml::chooseReplace( int partitionIndex )
{
    if ( !encryptionValid() )
    {
        return false;
    }

    Partition* partition = selectedPartition( partitionIndex );
    if ( !partition || !applyReplace( partition ) )
    {
        return false;
    }

    m_config->setInstallChoice( Config::InstallChoice::Replace );
    m_selectedPartitionPath = partition->partitionPath();
    Q_EMIT installChoiceChanged();
    Q_EMIT choiceApplied();
    updateNextStatus();
    return true;
}

void
ChoicePageQml::clearChoice()
{
    if ( m_core && m_core->isDirty() )
    {
        m_core->revertAllDevices();
        refreshPartitionModel();
    }

    m_config->setInstallChoice( Config::InstallChoice::NoChoice );
    m_selectedPartitionPath.clear();
    Q_EMIT installChoiceChanged();
    updateNextStatus();
}

QVariantList
ChoicePageQml::swapChoices() const
{
    QVariantList result;
    if ( !m_config )
    {
        return result;
    }

    for ( const Config::SwapChoice choice : m_config->swapChoices() )
    {
        QVariantMap item;
        item.insert( QStringLiteral( "value" ), static_cast< int >( choice ) );

        switch ( choice )
        {
        case Config::SwapChoice::NoSwap:
            item.insert( QStringLiteral( "label" ), tr( "No swap" ) );
            break;
        case Config::SwapChoice::ReuseSwap:
            item.insert( QStringLiteral( "label" ), tr( "Reuse swap" ) );
            break;
        case Config::SwapChoice::SmallSwap:
            item.insert( QStringLiteral( "label" ), tr( "Swap without hibernation" ) );
            break;
        case Config::SwapChoice::FullSwap:
            item.insert( QStringLiteral( "label" ), tr( "Swap with hibernation" ) );
            break;
        case Config::SwapChoice::SwapFile:
            item.insert( QStringLiteral( "label" ), tr( "Swap file" ) );
            break;
        }

        result.append( item );
    }

    return result;
}

void
ChoicePageQml::setSwapChoice( int choice )
{
    if ( !m_config )
    {
        return;
    }

    m_config->setSwapChoice( choice );
    if ( m_config->installChoice() == Config::InstallChoice::Erase )
    {
        chooseErase();
    }
}

Device*
ChoicePageQml::selectedDevice() const
{
    if ( !m_core || !m_core->deviceModel() )
    {
        return nullptr;
    }

    return m_core->deviceModel()->deviceForIndex( m_core->deviceModel()->index( m_deviceIndex, 0 ) );
}

Partition*
ChoicePageQml::selectedPartition( int index ) const
{
    return m_partitionModel ? m_partitionModel->partitionAt( index ) : nullptr;
}

void
ChoicePageQml::refreshPartitionModel()
{
    delete m_partitionModel;
    m_partitionModel = nullptr;

    Device* device = selectedDevice();
    if ( device )
    {
        auto* sourceModel = m_core->partitionModelForDevice( device );
        if ( sourceModel )
        {
            m_partitionModel = new FlatPartitionModel( sourceModel, this );
        }
    }

    Q_EMIT partitionModelChanged();
}

void
ChoicePageQml::updateNextStatus()
{
    Q_EMIT nextStatusChanged( isNextEnabled() );
}

void
ChoicePageQml::reapplyChoice()
{
    if ( !m_core || !m_config || !encryptionValid() )
    {
        if ( m_config && m_config->installChoice() != Config::InstallChoice::NoChoice )
        {
            clearChoice();
        }
        return;
    }

    if ( m_config->installChoice() == Config::InstallChoice::Erase )
    {
        if ( applyErase() )
        {
            Q_EMIT choiceApplied();
        }
        else
        {
            clearChoice();
        }
    }
    else if ( m_config->installChoice() == Config::InstallChoice::Replace )
    {
        Device* device = selectedDevice();
        Partition* partition = device && !m_selectedPartitionPath.isEmpty()
            ? Calamares::Partition::findPartitionByPath( { device }, m_selectedPartitionPath )
            : nullptr;
        if ( partition && applyReplace( partition ) )
        {
            Q_EMIT choiceApplied();
        }
        else
        {
            clearChoice();
        }
    }
}

bool
ChoicePageQml::applyErase()
{
    auto* device = selectedDevice();
    auto* queue = Calamares::JobQueue::instance();
    auto* globalStorage = queue ? queue->globalStorage() : nullptr;
    if ( !device || !globalStorage )
    {
        return false;
    }

    if ( m_core->isDirty() )
    {
        m_core->revertAllDevices();
        device = selectedDevice();
        if ( !device )
        {
            return false;
        }
    }

    bool storageOk = false;
    const double requiredStorageGiB
        = globalStorage->value( QStringLiteral( "requiredStorageGiB" ) ).toDouble( &storageOk );
    constexpr double maxGiB
        = static_cast< double >( std::numeric_limits< qint64 >::max() ) / ( 1024.0 * 1024.0 * 1024.0 );
    if ( !storageOk || !std::isfinite( requiredStorageGiB ) || requiredStorageGiB < 0.0
         || requiredStorageGiB > maxGiB - 0.5 )
    {
        return false;
    }

    const PartitionActions::Choices::AutoPartitionOptions options {
        globalStorage->value( QStringLiteral( "defaultPartitionTableType" ) ).toString(),
        m_config->eraseFsType(),
        m_config->luksFileSystemType(),
        m_encryptionChecked ? m_encryptPassphrase : QString(),
        globalStorage->value( QStringLiteral( "efiSystemPartition" ) ).toString(),
        Calamares::GiBtoBytes( requiredStorageGiB ),
        m_config->swapChoice(),
    };

    PartitionActions::doAutopartition( m_core, device, options );
    return true;
}

bool
ChoicePageQml::applyReplace( Partition* partition )
{
    auto* device = selectedDevice();
    auto* queue = Calamares::JobQueue::instance();
    auto* globalStorage = queue ? queue->globalStorage() : nullptr;
    if ( !device || !partition || !globalStorage )
    {
        return false;
    }

    if ( !PartUtils::canBeReplaced( partition, Logger::Once() ) )
    {
        return false;
    }

    const QString partitionPath = partition->partitionPath();
    if ( m_core->isDirty() )
    {
        m_core->revertDevice( device );
        device = selectedDevice();
        if ( !device )
        {
            return false;
        }
        partition = Calamares::Partition::findPartitionByPath( { device }, partitionPath );
        if ( !partition || !PartUtils::canBeReplaced( partition, Logger::Once() ) )
        {
            return false;
        }
    }

    const PartitionActions::Choices::ReplacePartitionOptions options {
        globalStorage->value( QStringLiteral( "defaultPartitionTableType" ) ).toString(),
        m_config->replaceModeFilesystem(),
        m_config->luksFileSystemType(),
        m_encryptionChecked ? m_encryptPassphrase : QString(),
    };

    PartitionActions::doReplacePartition( m_core, device, partition, options );
    return true;
}
