/* === This file is part of Calamares - <https://calamares.io> ===
 *
 *   SPDX-FileCopyrightText: 2026 Uri Herrera <uri@nxos.org>
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 *   Calamares is Free Software: see the License-Identifier above.
 *
 */

#include "PartitionQmlViewStep.h"

#include "ChoicePageQml.h"
#include "Config.h"
#include "core/PartitionCoreModule.h"

#include "GlobalStorage.h"
#include "JobQueue.h"

#include "utils/Variant.h"

#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>

CALAMARES_PLUGIN_FACTORY_DEFINITION( PartitionQmlViewStepFactory, registerPlugin< PartitionQmlViewStep >(); )

PartitionQmlViewStep::PartitionQmlViewStep( QObject* parent )
    : Calamares::QmlViewStep( parent )
    , m_config( new Config( this ) )
    , m_choicePage( new ChoicePageQml( m_config, this ) )
    , m_core( new PartitionCoreModule( this ) )
{
    setContextProperty( "core", m_core );
    setContextProperty( "choicePage", m_choicePage );

    connect( m_choicePage, &ChoicePageQml::nextStatusChanged, this, &PartitionQmlViewStep::nextStatusChanged );
}

PartitionQmlViewStep::~PartitionQmlViewStep()
{
    if ( m_future )
    {
        m_future->waitForFinished();
    }
}

QString
PartitionQmlViewStep::prettyName() const
{
    return tr( "Partitions", "@label" );
}

bool
PartitionQmlViewStep::isNextEnabled() const
{
    return m_choicePage->isNextEnabled();
}

bool
PartitionQmlViewStep::isBackEnabled() const
{
    return true;
}

bool
PartitionQmlViewStep::isAtBeginning() const
{
    return true;
}

bool
PartitionQmlViewStep::isAtEnd() const
{
    return m_choicePage->isAtEnd();
}

Calamares::JobList
PartitionQmlViewStep::jobs() const
{
    return m_coreReady ? m_core->jobs( m_config ) : Calamares::JobList();
}

void
PartitionQmlViewStep::onActivate()
{
    m_config->fillGSSecondaryConfiguration();
    if ( !m_coreReady )
    {
        return;
    }
    m_choicePage->initialize( m_core );
}

void
PartitionQmlViewStep::onLeave()
{
}

void
PartitionQmlViewStep::setConfigurationMap( const QVariantMap& configurationMap )
{
    if ( m_configurationSet )
    {
        return;
    }
    m_configurationSet = true;

    m_config->setConfigurationMap( configurationMap );

    auto* globalStorage = Calamares::JobQueue::instance()->globalStorage();
    if ( configurationMap.contains( "swapPartitionName" ) )
    {
        globalStorage->insert( "swapPartitionName", Calamares::getString( configurationMap, "swapPartitionName" ) );
    }
    globalStorage->insert( "drawNestedPartitions", Calamares::getBool( configurationMap, "drawNestedPartitions", false ) );
    globalStorage->insert( "alwaysShowPartitionLabels", Calamares::getBool( configurationMap, "alwaysShowPartitionLabels", true ) );
    const bool enableLuksAutomatedPartitioning
        = Calamares::getBool( configurationMap, "enableLuksAutomatedPartitioning", true );
    globalStorage->insert( "enableLuksAutomatedPartitioning", enableLuksAutomatedPartitioning );
    m_choicePage->setEncryptionEnabled( enableLuksAutomatedPartitioning );
    globalStorage->insert( "defaultPartitionTableType", Calamares::getString( configurationMap, "defaultPartitionTableType" ) );
    globalStorage->insert( "createHybridBootloaderLayout", Calamares::getBool( configurationMap, "createHybridBootloaderLayout", false ) );

    m_core->partitionLayout().init( m_config->defaultFsType(), configurationMap.value( "partitionLayout" ).toList() );
    m_core->dirFSRestrictLayout().init( configurationMap.value( "directoryFilesystemRestrictions" ).toList() );

    Calamares::QmlViewStep::setConfigurationMap( configurationMap );

    if ( m_future )
    {
        return;
    }

    m_future = new QFutureWatcher<void>( this );
    connect( m_future,
             &QFutureWatcher<void>::finished,
             this,
             [ this ]
             {
                 m_coreReady = true;
                 m_choicePage->initialize( m_core );
                 m_future->deleteLater();
                 m_future = nullptr;
             } );

    m_future->setFuture( QtConcurrent::run( &PartitionCoreModule::init, m_core ) );
}

QObject*
PartitionQmlViewStep::getConfig()
{
    return m_config;
}
