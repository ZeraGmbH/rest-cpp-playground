#include "veinentry.h"
#include "task_client_component_setter.h"
#include "task_client_rpc_invoker.h"
#include "tasklambdarunner.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <qjsonarray.h>

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
        QVariant valueToBeSent = value;
        if (static_cast<QMetaType::Type>(m_storage.getDb()->getStoredValue(entityId, componentName).type()) == QMetaType::QJsonObject || static_cast<QMetaType::Type>(m_storage.getDb()->getStoredValue(entityId, componentName).type()) == QMetaType::QVariantMap) {
            QString data(valueToBeSent.toString());
            QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8());
            if (doc.isArray())
                valueToBeSent = doc.array();
            else
                valueToBeSent = doc.object();

            if (valueToBeSent.isNull())
                return TaskLambdaRunner::create([](){return false;});
        }
        TaskTemplatePtr setter = TaskClientComponentSetter::create(entityItem, componentName, m_storage.getDb()->getStoredValue(entityId, componentName), valueToBeSent, 1000, []() {
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

std::shared_ptr<TaskTemplate> VeinEntry::rpcToVein(int entityId, QString rpc_name, QVariantMap parameters, std::shared_ptr<QVariant> result, int timeout)
{
    if(m_storage.getDb()->hasEntity(entityId)) {
        TaskTemplatePtr task = TaskClientRPCInvoker::create(entityId, rpc_name, parameters, result, m_cmdEventHandlerSystem, timeout, []() {
            qWarning("Task failed");
        });
        std::shared_ptr<TaskTemplate> taskSharedPtr = std::move(task);
        connect(taskSharedPtr.get(), &TaskTemplate::sigFinish, this, [taskSharedPtr](bool ok, int taskId){
        });
        return taskSharedPtr;
    }
    else
        return TaskSimpleVeinRPCInvoker::create(entityId, rpc_name, parameters, result, m_cmdEventHandlerSystem, timeout);
}

VeinStorage::AbstractDatabase *VeinEntry::getStorageDb()
{
    return m_storage.getDb();
}

std::shared_ptr<SubscriptionManager> VeinEntry::getSubscriptionManager()
{
    return m_subscriptionManager;
}

QVariant VeinEntry::jsonToVariant(const QString &json){
    // Wrap JSON serialized value in a dummy object do simplify parsing.
    QString docValue = QString("{\"d\":%1}").arg(json);

    // Parse the dummy object.
    QJsonDocument doc(QJsonDocument::fromJson(docValue.toUtf8()));

    // Copy the original value deserialized as a variant to the RPC actual parameter map.
    return doc["d"].toVariant();
}
