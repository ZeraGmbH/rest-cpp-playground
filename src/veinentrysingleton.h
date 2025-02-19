#ifndef VEINENTRYSINGLETON_H
#define VEINENTRYSINGLETON_H

#include "commondefines.h"
#include <QObject>
#include <ve_eventhandler.h>
#include <vn_networksystem.h>
#include <vn_tcpsystem.h>
#include <tcpnetworkfactory.h>
#include <vs_clientstorageeventsystem.h>
#include <vf_cmd_event_handler_system.h>
#include <tasksimpleveingetter.h>
#include <tasksimpleveinsetter.h>

class VeinEntrySingleton : public QObject
{
    Q_OBJECT

public:
    static VeinEntrySingleton& getInstance(VeinTcp::AbstractTcpNetworkFactoryPtr tcpNetworkFactory = VeinTcp::TcpNetworkFactory::create())
    {
        static VeinEntrySingleton instance(tcpNetworkFactory);

        return instance;
    }

    TaskSimpleVeinGetterPtr getFromVein(int entityId, QString componentName);
    TaskSimpleVeinSetterPtr setToVein(int entityId, QString componentName, QVariant value);

    VeinStorage::AbstractDatabase* getStorageDb();

    VeinEntrySingleton(VeinEntrySingleton const&)   = delete;
    void operator=(VeinEntrySingleton const&)       = delete;

signals:
    void sigSubscriberTasksFinish(bool ok);

private:
    VeinEntrySingleton(VeinTcp::AbstractTcpNetworkFactoryPtr tcpNetworkFactory);
    TaskTemplatePtr createSubscriptionTask(veinRestEntityIds entityId, const QString& entityName);

    VeinEvent::EventHandler m_eventHandler;
    VeinNet::NetworkSystem m_netSystem;
    VeinNet::TcpSystem m_tcpSystem;
    VeinStorage::ClientStorageEventSystem m_storage;
    VfCmdEventHandlerSystemPtr m_cmdEventHandlerSystem;
    std::shared_ptr<QStringList> m_dummyComponentList;
    TaskContainerInterfacePtr m_subscriberTask;


};

#endif // VEINENTRYSINGLETON_H
