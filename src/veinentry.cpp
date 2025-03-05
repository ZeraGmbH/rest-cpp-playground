#include "veinentry.h"
#include "task_client_component_setter.h"

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

TaskTemplatePtr VeinEntry::setToVein(int entityId, QString componentName, QVariant value)
{
    /* To set a component the old value needs to be known.
     * If the storage does not have the old value, execute a full setter task with subscribe->get->set
     * If it has the old value, only execute a "set", with the old value from the storage, as subscribing again will make the storage error out
    */
    if(m_storage.getDb()->hasEntity(entityId)) {
        VfCmdEventItemEntityPtr entityItem = VfEntityComponentEventItem::create(entityId);
        m_cmdEventHandlerSystem->addItem(entityItem);
        TaskTemplatePtr setter = TaskClientComponentSetter::create(entityItem, componentName, m_storage.getDb()->getStoredValue(entityId, componentName), value, 1000, []() {
            qWarning("Setter Task failed");
        });
        connect(setter.get(), &TaskTemplate::sigFinish, this, [this, entityItem](){
            m_cmdEventHandlerSystem->removeItem(entityItem);
        });
        return setter;
    }
    else
    {
        TaskSimpleVeinSetterPtr task = TaskSimpleVeinSetter::create(entityId, componentName, value, m_cmdEventHandlerSystem);

        return task;
    }
}

VeinStorage::AbstractDatabase *VeinEntry::getStorageDb()
{
    return m_storage.getDb();
}

std::shared_ptr<SubscriptionManager> VeinEntry::getSubscriptionManager()
{
    return m_subscriptionManager;
}
