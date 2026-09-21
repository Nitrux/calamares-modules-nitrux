/* === This file is part of Calamares - <https://calamares.io> ===
 *
 *   SPDX-FileCopyrightText: 2026 Uri Herrera <uri@nxos.org>
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 *   Calamares is Free Software: see the License-Identifier above.
 *
 */

#ifndef NITRUX_CHOICEPAGEQML_H
#define NITRUX_CHOICEPAGEQML_H

#include <QAbstractItemModel>
#include <QObject>
#include <QVariantList>

class Config;
class Device;
class FlatPartitionModel;
class Partition;
class PartitionCoreModule;

class ChoicePageQml final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel* deviceModel READ deviceModel NOTIFY deviceModelChanged)
    Q_PROPERTY(QAbstractItemModel* partitionModel READ partitionModel NOTIFY partitionModelChanged)
    Q_PROPERTY(int currentDeviceIndex READ currentDeviceIndex WRITE setCurrentDeviceIndex NOTIFY currentDeviceIndexChanged)
    Q_PROPERTY(int installChoice READ installChoice NOTIFY installChoiceChanged)
    Q_PROPERTY(bool ready READ ready NOTIFY readyChanged)
    Q_PROPERTY(bool eraseEnabled READ eraseEnabled NOTIFY readyChanged)
    Q_PROPERTY(bool replaceEnabled READ replaceEnabled NOTIFY partitionModelChanged)
    Q_PROPERTY(bool alongsideEnabled READ alongsideEnabled CONSTANT)
    Q_PROPERTY(bool manualEnabled READ manualEnabled CONSTANT)
    Q_PROPERTY(QString currentDeviceName READ currentDeviceName NOTIFY currentDeviceIndexChanged)
    Q_PROPERTY(bool encryptionEnabled READ encryptionEnabled NOTIFY encryptionEnabledChanged)
    Q_PROPERTY(bool encryptionChecked READ encryptionChecked WRITE setEncryptionChecked NOTIFY encryptionCheckedChanged)
    Q_PROPERTY(QString encryptionPassphrase READ encryptionPassphrase WRITE setEncryptionPassphrase NOTIFY encryptionPassphraseChanged)
    Q_PROPERTY(QString encryptionPassphraseConfirmation READ encryptionPassphraseConfirmation WRITE setEncryptionPassphraseConfirmation NOTIFY encryptionPassphraseConfirmationChanged)
    Q_PROPERTY(bool encryptionValid READ encryptionValid NOTIFY encryptionValidityChanged)

public:
    enum InstallChoice
    {
        NoChoice = 0,
        Alongside = 1,
        Erase = 2,
        Replace = 3,
        Manual = 4,
    };
    Q_ENUM(InstallChoice)

    explicit ChoicePageQml( Config* config, QObject* parent = nullptr );

    void initialize( PartitionCoreModule* core );

    QAbstractItemModel* deviceModel() const;
    QAbstractItemModel* partitionModel() const;
    int currentDeviceIndex() const;
    void setCurrentDeviceIndex( int index );
    int installChoice() const;
    bool ready() const;
    bool eraseEnabled() const;
    bool replaceEnabled() const;
    bool alongsideEnabled() const;
    bool manualEnabled() const;
    QString currentDeviceName() const;
    bool encryptionEnabled() const;
    bool encryptionChecked() const;
    void setEncryptionChecked( bool checked );
    QString encryptionPassphrase() const;
    void setEncryptionPassphrase( const QString& passphrase );
    QString encryptionPassphraseConfirmation() const;
    void setEncryptionPassphraseConfirmation( const QString& passphrase );
    bool encryptionValid() const;

    void setEncryptionEnabled( bool enabled );

    bool isNextEnabled() const;
    bool isAtEnd() const;

    Q_INVOKABLE bool chooseErase();
    Q_INVOKABLE bool chooseReplace( int partitionIndex );
    Q_INVOKABLE void clearChoice();
    Q_INVOKABLE QVariantList swapChoices() const;
    Q_INVOKABLE void setSwapChoice( int choice );

Q_SIGNALS:
    void deviceModelChanged();
    void partitionModelChanged();
    void currentDeviceIndexChanged();
    void installChoiceChanged();
    void readyChanged();
    void encryptionEnabledChanged();
    void encryptionCheckedChanged();
    void encryptionPassphraseChanged();
    void encryptionPassphraseConfirmationChanged();
    void encryptionValidityChanged();
    void nextStatusChanged( bool enabled );
    void choiceApplied();

private:
    Device* selectedDevice() const;
    Partition* selectedPartition( int index ) const;
    void refreshPartitionModel();
    void updateNextStatus();
    void reapplyChoice();
    bool applyErase();
    bool applyReplace( Partition* partition );

    Config* m_config;
    PartitionCoreModule* m_core = nullptr;
    FlatPartitionModel* m_partitionModel = nullptr;
    int m_deviceIndex = -1;
    bool m_ready = false;
    bool m_encryptionEnabled = false;
    bool m_encryptionChecked = false;
    QString m_encryptPassphrase;
    QString m_encryptPassphraseConfirmation;
    QString m_selectedPartitionPath;
};

#endif
