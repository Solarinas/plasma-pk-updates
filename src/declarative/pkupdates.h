/***************************************************************************
 *   Copyright (C) 2015 Lukáš Tinkl <lukas@kde.org>                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; see the file COPYING. If not, write to       *
 *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,  *
 *   Boston, MA 02110-1301, USA.                                           *
 ***************************************************************************/


#ifndef PLASMA_PK_UPDATES_H
#define PLASMA_PK_UPDATES_H

#include <QObject>
#include <QPointer>
#include <QLoggingCategory>

#include <PackageKit/Daemon>
#include <PackageKit/Transaction>

class QTimer;
class KNotification;

Q_DECLARE_LOGGING_CATEGORY(PLASMA_PK_UPDATES)

/**
 * @brief The PkUpdates class
 *
 * Backend class to check for available PackageKit system updates.
 * Use checkUpdates() to perform the check, retrieve them with packages()
 */
class PkUpdates : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY updatesChanged)
    Q_PROPERTY(int importantCount READ importantCount NOTIFY updatesChanged)
    Q_PROPERTY(int securityCount READ securityCount NOTIFY updatesChanged)
    Q_PROPERTY(bool isSystemUpToDate READ isSystemUpToDate NOTIFY updatesChanged)
    Q_PROPERTY(QString iconName READ iconName NOTIFY updatesChanged)
    Q_PROPERTY(QString message READ message NOTIFY messageChanged)
    Q_PROPERTY(int percentage READ percentage NOTIFY percentageChanged)
    Q_PROPERTY(QString timestamp READ timestamp NOTIFY updatesChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(QVariantMap packages READ packages NOTIFY updatesChanged)
    Q_PROPERTY(bool isActive READ isActive NOTIFY isActiveChanged)
    Q_PROPERTY(bool isNetworkOnline READ isNetworkOnline NOTIFY networkStateChanged)
    Q_PROPERTY(bool isNetworkMobile READ isNetworkMobile NOTIFY networkStateChanged)
    Q_PROPERTY(bool isOnBattery READ isOnBattery NOTIFY isOnBatteryChanged)

public:
    enum Activity {Idle, CheckingUpdates, GettingUpdates, InstallingUpdates};
    Q_ENUM(Activity)

    explicit PkUpdates(QObject *parent = nullptr);
    ~PkUpdates();

    /**
     * @return the total number of updates, including important and/or security ones
     */
    int count() const;

    /**
     * @return the number of important updates, included in total count()
     */
    int importantCount() const;

    /**
     * @return the number of security updates, included in total count()
     */
    int securityCount() const;

    /**
     * @return whether the system is up to date (count() == 0)
     */
    bool isSystemUpToDate() const;

    /**
     * @return the system update status icon name
     */
    QString iconName() const;

    /**
     * @return the overall status with number of available updates
     */
    QString message() const;

    /**
     * @return the progress percentage (0..100), 101 as a special value indicating indeterminate value
     */
    int percentage() const;

    /**
     * @return time stamp of the last update check
     */
    QString timestamp() const;

    /**
     * @return status messsage conveying the action being currently performed
     */
    QString statusMessage() const;

    /**
     * @return whether we're currently checking for updates or not
     */
    bool isActive() const;

    /**
     * @return the packages to update (key=packageId, value=description)
     */
    QVariantMap packages() const;

    /**
     * @return whether the network is online
     */
    bool isNetworkOnline() const;

    /**
     * @return whether we are on a mobile network connection (assumes isNetworkOnline())
     */
    bool isNetworkMobile() const;

    /**
     * @return whether we are running on battery
     */
    bool isOnBattery() const;

signals:
    /**
     * Emitted when the number uf updates has changed
     */
    void updatesChanged();

    /**
     * Emitted when the updates check is finished (with success or error)
     */
    void done();

    /**
     * Emitted after updates were installed successfully
     */
    void updatesInstalled();

    /**
     * Emitted with update details
     * @see getUpdateDetails()
     */
    void updateDetail(const QString &packageID, const QString &updateText, const QStringList &urls);

    /**
     * Emitted when an EULA agreement prevents the transaction from running
     * @param eulaId the EULA identifier
     * @param packageID ID of the package for which an EULA is required
     * @param vendorName the vendor name
     * @param licenseAgreement the EULA text
     *
     * @see eulaAgreementResult()
     */
    void eulaRequired(const QString &eulaID, const QString &packageID, const QString &vendor, const QString &licenseAgreement);

    // private ;)
    void statusMessageChanged();
    void isActiveChanged();
    void percentageChanged();
    void networkStateChanged();
    void isOnBatteryChanged();
    void messageChanged();

public slots:
    /**
      * Perform a cache update, possibly resulting in an update check. Signal updatesChanged() gets emitted
      * as a result. Consult the count() property whether there are new updates available.
      *
      * @param force whether to force the cache refresh
      * @param manual whether this check was triggered via explicit user interaction
      */
    Q_INVOKABLE void checkUpdates(bool force = true, bool manual = false);

    /**
      * Launch the update process
      *
      * @param packageIds list of package IDs to update
      */
    Q_INVOKABLE void installUpdates(const QStringList & packageIds, bool simulate = true, bool untrusted = false);

    /**
      * @return the timestamp (in milliseconds) of the last cache check, -1 if never
      */
    Q_INVOKABLE qint64 lastRefreshTimestamp() const;

    /**
      * @return the package name extracted from its ID
      */
    Q_INVOKABLE static QString packageName(const QString & pkgId);

    /**
      * @return the package version extracted from its ID
      */
    Q_INVOKABLE static QString packageVersion(const QString & pkgId);

    /**
     * Request details about the details
     * @param pkgIds Package IDs
     *
     * Emits updateDetail()
     */
    Q_INVOKABLE void getUpdateDetails(const QString & pkgID);

    Q_INVOKABLE void doDelayedCheckUpdates();

    /**
     * If agreed to eulaID, starts an EULA acceptance transaction and continues.
     */
    Q_INVOKABLE void eulaAgreementResult(const QString &eulaID, bool agreed);

private slots:
    void getUpdates();
    void onChanged();
    void onUpdatesChanged();
    void onStatusChanged();
    void onPackage(PackageKit::Transaction::Info info, const QString &packageID, const QString &summary);
    void onPackageUpdating(PackageKit::Transaction::Info info, const QString &packageID, const QString &summary);
    void onFinished(PackageKit::Transaction::Exit status, uint runtime);
    void onErrorCode(PackageKit::Transaction::Error error, const QString &details);
    void onRefreshErrorCode(PackageKit::Transaction::Error error, const QString &details);
    void onRequireRestart(PackageKit::Transaction::Restart type, const QString &packageID);
    void onUpdateDetail(const QString &packageID, const QStringList &updates, const QStringList &obsoletes, const QStringList &vendorUrls,
                        const QStringList &bugzillaUrls, const QStringList &cveUrls, PackageKit::Transaction::Restart restart,
                        const QString &updateText, const QString &changelog, PackageKit::Transaction::UpdateState state,
                        const QDateTime &issued, const QDateTime &updated);
    void onRepoSignatureRequired(const QString & packageID, const QString & repoName, const QString & keyUrl, const QString & keyUserid,
                                 const QString & keyId, const QString & keyFingerprint, const QString & keyTimestamp, PackageKit::Transaction::SigType type);
    void onEulaRequired(const QString &eulaID, const QString &packageID, const QString &vendor, const QString &licenseAgreement);

private:
    struct EulaData {
        QString packageID;
        QString vendor;
        QString licenseAgreement;
    };

    void setStatusMessage(const QString &message);
    void setActivity(Activity act);
    void setPercentage(int value);
    void showError(PackageKit::Transaction::Error error, const QString &details);
    void promptNextEulaAgreement();
    QPointer<PackageKit::Transaction> m_updatesTrans;
    QPointer<PackageKit::Transaction> m_cacheTrans;
    QPointer<PackageKit::Transaction> m_installTrans;
    QPointer<PackageKit::Transaction> m_detailTrans;
    QPointer<PackageKit::Transaction> m_eulaTrans;
    QStringList m_packages;
    QPointer<KNotification> m_lastNotification;
    int m_lastUpdateCount = 0;
    QVariantMap m_updateList;
    QStringList m_importantList;
    QStringList m_securityList;
    QString m_statusMessage;
    int m_percentage = 0;
    Activity m_activity = Idle;
    enum {
        NoCheckDone,
        CheckFailed,
        CheckSucceeded
    } m_lastCheckState = NoCheckDone;
    bool m_checkUpdatesWhenNetworkOnline = false;
    bool m_isOnBattery;
    // If the current check was triggered manually
    bool m_isManualCheck;
    // If a transaction failed because of required EULAs,
    // this contains a map of their IDs to their data
    QMap<QString, EulaData> m_requiredEulas;
};

#endif // PLASMA_PK_UPDATES_H
