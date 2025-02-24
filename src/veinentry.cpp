#include "veinentry.h"

std::shared_ptr<VeinEntry> VeinEntry::create(VeinTcp::AbstractTcpNetworkFactoryPtr tcpNetworkFactory)
{
    return std::make_shared<VeinEntry>(tcpNetworkFactory);
}

VeinEntry::VeinEntry(VeinTcp::AbstractTcpNetworkFactoryPtr tcpNetworkFactory) :
    m_tcpSystem(tcpNetworkFactory),
    m_cmdEventHandlerSystem(VfCmdEventHandlerSystem::create()),
    m_subscriptionManager(std::make_shared<SubscriptionManager>(m_cmdEventHandlerSystem))
{
    m_eventHandler.addSubsystem(&m_netSystem);
    m_eventHandler.addSubsystem(&m_tcpSystem);
    m_eventHandler.addSubsystem(&m_storage);
    m_eventHandler.addSubsystem(m_cmdEventHandlerSystem.get());
    m_netSystem.setOperationMode(VeinNet::NetworkSystem::VNOM_PASS_THROUGH);

    m_tcpSystem.connectToServer("127.0.0.1", 12000);

    connect(&m_tcpSystem, &VeinNet::TcpSystem::sigConnnectionEstablished, m_subscriptionManager.get(), &SubscriptionManager::startObservingSessionChange);
}

TaskSimpleVeinSetterPtr VeinEntry::setToVein(int entityId, QString componentName, QVariant value)
{
    TaskSimpleVeinSetterPtr task = TaskSimpleVeinSetter::create(entityId, componentName, value, m_cmdEventHandlerSystem);

    return task;
}

VeinStorage::AbstractDatabase *VeinEntry::getStorageDb()
{
    return m_storage.getDb();
}

std::shared_ptr<SubscriptionManager> VeinEntry::getSubscriptionManager()
{
    return m_subscriptionManager;
}
