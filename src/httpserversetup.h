#ifndef HTTPSERVERSETUP_H
#define HTTPSERVERSETUP_H

#include "OAIApiRouter.h"
#include <qhttpengine/server.h>
#include <QObject>

class HttpServerSetup : public QObject
{
    Q_OBJECT
public:
    HttpServerSetup(VeinTcp::AbstractTcpNetworkFactoryPtr tcpNetworkFactory);
    bool startListening(const QHostAddress &address, quint16 port);
private:
    QHttpEngine::Server m_server;
    std::shared_ptr<OpenAPI::OAIApiRequestHandler> m_handler;
    VeinEntryPtr m_veinEntry;
    std::shared_ptr<OpenAPI::OAIApiRouter> m_router;
};

#endif // HTTPSERVERSETUP_H
