/* === This file is part of Calamares - <https://calamares.io> ===
 *
 *   SPDX-FileCopyrightText: 2026 Uri Herrera <uri@nxos.org>
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 *   Calamares is Free Software: see the License-Identifier above.
 *
 */

#ifndef NITRUX_PARTITIONQMLVIEWSTEP_H
#define NITRUX_PARTITIONQMLVIEWSTEP_H

#include "DllMacro.h"
#include "utils/PluginFactory.h"
#include "viewpages/QmlViewStep.h"

#include <QObject>

class Config;
class ChoicePageQml;
class PartitionCoreModule;

template <typename T>
class QFutureWatcher;

class PLUGINDLLEXPORT PartitionQmlViewStep final : public Calamares::QmlViewStep
{
    Q_OBJECT

public:
    explicit PartitionQmlViewStep( QObject* parent = nullptr );
    ~PartitionQmlViewStep() override;

    QString prettyName() const override;
    bool isNextEnabled() const override;
    bool isBackEnabled() const override;
    bool isAtBeginning() const override;
    bool isAtEnd() const override;

    Calamares::JobList jobs() const override;

    void onActivate() override;
    void onLeave() override;
    void setConfigurationMap( const QVariantMap& configurationMap ) override;

protected:
    QObject* getConfig() override;

private:
    Config* m_config;
    ChoicePageQml* m_choicePage;
    PartitionCoreModule* m_core;
    QFutureWatcher<void>* m_future = nullptr;
    bool m_coreReady = false;
    bool m_configurationSet = false;
};

CALAMARES_PLUGIN_FACTORY_DECLARATION( PartitionQmlViewStepFactory )

#endif
