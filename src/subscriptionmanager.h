#ifndef SUBSCRIPTIONMANAGER_H
#define SUBSCRIPTIONMANAGER_H

#include "taskcontainerinterface.h"
#include "vf_cmd_event_handler_system.h"
#include "vf_component_change_notifier.h"
#include <QObject>

class SubscriptionManager : public QObject
{
    Q_OBJECT
public:
    SubscriptionManager(VfCmdEventHandlerSystemPtr eventHandler);
    bool hasEntity(int entityId);
    void subscribeToEntities(QList<int> entities);
    void clearSubscriptions();
public slots:
    void startObservingSessionChange(QUuid id);

signals:
    void finishedSubscribing(bool ok);

private slots:
    void onTaskFinished(bool ok, int taskId);
    void onSessionChange();

private:
    QList<int> m_subscribedEntities;
    QList<int> m_subscriptionsInProgress;
    TaskContainerInterfacePtr m_subscriberTask;
    VfCmdEventHandlerSystemPtr m_eventHandler;
    std::shared_ptr<VfComponentChangeNotifier> m_sessionNotifier;
    QString m_currentSession;
};

#endif // SUBSCRIPTIONMANAGER_H
