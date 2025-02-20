#include "subscriptionmanager.h"
#include "taskcontainerparallel.h"
#include "tasksimpleveinentitygetter.h"
#include "tasklambdarunner.h"

SubscriptionManager::SubscriptionManager(VfCmdEventHandlerSystemPtr eventHandler) :
    m_eventHandler(eventHandler),
    m_subscriberTask(TaskContainerParallel::create()),
    m_subscribedEntities(QList<int>()),
    m_subscriptionsInProgress(QList<int>())
{
    connect(m_subscriberTask.get(), &TaskTemplate::sigFinish, this, &SubscriptionManager::onTaskFinished);
}

bool SubscriptionManager::hasEntity(int entityId)
{
    return m_subscribedEntities.contains(entityId);
}

void SubscriptionManager::subscribeToEntities(QList<int> entities)
{
    bool empty = true;
    for (const auto item : entities)
    {
        if (!m_subscribedEntities.contains(item)) {
            TaskTemplatePtr getterTask = TaskSimpleVeinEntityGetter::create(item, m_eventHandler, 2000);
            m_subscriberTask->addSub(std::move(getterTask));
            m_subscriptionsInProgress.append(item);
            empty = false;
        }
    }
    if (empty)
    {
        TaskTemplatePtr dummyTask = TaskLambdaRunner::create([](){return true;});
        m_subscriberTask->addSub(std::move(dummyTask));
    }

    m_subscriberTask->start();
}

void SubscriptionManager::clearSubscriptions()
{
    m_subscribedEntities.clear();
}

void SubscriptionManager::startObservingSessionChange(QUuid id)
{
    Q_UNUSED(id);
    VfCmdEventItemEntityPtr eventItem = VfEntityComponentEventItem::create(0);
    m_eventHandler->addItem(eventItem);
    m_sessionNotifier = VfComponentChangeNotifier::create("Session", eventItem);
    eventItem->addItem(m_sessionNotifier);
    subscribeToEntities(QList<int>() << 0);
    connect(m_sessionNotifier.get(), &VfComponentChangeNotifier::sigValueChanged, this, &SubscriptionManager::onSessionChange);
}

void SubscriptionManager::onTaskFinished(bool ok, int taskId)
{
    m_subscribedEntities.append(m_subscriptionsInProgress);
    m_subscriptionsInProgress.clear();
    m_subscriberTask = TaskContainerParallel::create();
    connect(m_subscriberTask.get(), &TaskTemplate::sigFinish, this, &SubscriptionManager::onTaskFinished);
    emit finishedSubscribing(ok);
}

void SubscriptionManager::onSessionChange()
{
    QString newSession = m_sessionNotifier->getValue().toString();
    if(newSession != "" && newSession != m_currentSession ) clearSubscriptions();
}


