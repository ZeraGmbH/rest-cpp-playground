#ifndef VEINENTRY_H
#define VEINENTRY_H

#include "abstracttcpnetworkfactory.h"
#include "subscriptionmanager.h"
#include "tasksimpleveinsetter.h"
#include "tasksimpleveinrpcinvoker.h"
#include "ve_eventhandler.h"
#include "vf_cmd_event_handler_system.h"
#include "vn_networksystem.h"
#include "vn_tcpsystem.h"
#include "vs_clientstorageeventsystem.h"
#include <QObject>

class VeinEntry : public QObject
{
    Q_OBJECT

public:
    static std::shared_ptr<VeinEntry> create(VeinTcp::AbstractTcpNetworkFactoryPtr tcpNetworkFactory);
    VeinEntry(VeinTcp::AbstractTcpNetworkFactoryPtr tcpNetworkFactory);

    TaskTemplatePtr setToVein(int entityId, QString componentName, QVariant value);
    std::shared_ptr<TaskTemplate> rpcToVein(int entityId, QString rpc_name, QVariantMap parameters, std::shared_ptr<QVariant> result);

    VeinStorage::AbstractDatabase* getStorageDb();
    std::shared_ptr<SubscriptionManager> getSubscriptionManager();

private:
    VeinEvent::EventHandler m_eventHandler;
    VeinNet::NetworkSystem m_netSystem;
    VeinNet::TcpSystem m_tcpSystem;
    VeinStorage::ClientStorageEventSystem m_storage;
    VfCmdEventHandlerSystemPtr m_cmdEventHandlerSystem;
    std::shared_ptr<SubscriptionManager> m_subscriptionManager;
};

typedef std::shared_ptr<VeinEntry> VeinEntryPtr;

#endif // VEINENTRY_H
