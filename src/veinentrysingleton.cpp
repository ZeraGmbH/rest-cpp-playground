#include "veinentrysingleton.h"

#include <QAbstractEventDispatcher>
#include <QCoreApplication>
#include <taskcontainerparallel.h>

TaskSimpleVeinGetterPtr VeinEntrySingleton::getFromVein(int entityId, QString componentName)
{
    TaskSimpleVeinGetterPtr task = TaskSimpleVeinGetter::create(entityId, componentName, m_cmdEventHandlerSystem);

    return task;
}

TaskSimpleVeinSetterPtr VeinEntrySingleton::setToVein(int entityId, QString componentName, QVariant value)
{
    TaskSimpleVeinSetterPtr task = TaskSimpleVeinSetter::create(entityId, componentName, value, m_cmdEventHandlerSystem);

    return task;
}

std::shared_ptr<SubscriptionManager> VeinEntrySingleton::getSubscriptionManager()
{
    return m_subscriptionManager;
}

VeinStorage::AbstractDatabase *VeinEntrySingleton::getStorageDb()
{
    return m_storage.getDb();
}

VeinEntrySingleton::VeinEntrySingleton(VeinTcp::AbstractTcpNetworkFactoryPtr tcpNetworkFactory) :
    m_tcpSystem(tcpNetworkFactory),
    m_cmdEventHandlerSystem(VfCmdEventHandlerSystem::create()),
    m_subscriptionManager(std::make_shared<SubscriptionManager>(m_cmdEventHandlerSystem))
{
    m_eventHandler.addSubsystem(&m_netSystem);
    m_eventHandler.addSubsystem(&m_tcpSystem);
    m_eventHandler.addSubsystem(m_cmdEventHandlerSystem.get());
    m_eventHandler.addSubsystem(&m_storage);
    m_netSystem.setOperationMode(VeinNet::NetworkSystem::VNOM_PASS_THROUGH);

    m_tcpSystem.connectToServer("127.0.0.1", 12000);

    connect(&m_tcpSystem, &VeinNet::TcpSystem::sigConnnectionEstablished, m_subscriptionManager.get(), &SubscriptionManager::startObservingSessionChange);
}


