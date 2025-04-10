#include "httpserversetup.h"
#include <systemd/sd-daemon.h>

HttpServerSetup::HttpServerSetup(VeinTcp::AbstractTcpNetworkFactoryPtr tcpNetworkFactory)
{
    m_handler = std::make_shared<OpenAPI::OAIApiRequestHandler>();
    m_veinEntry = VeinEntry::create(tcpNetworkFactory);
    m_router = std::make_shared<OpenAPI::OAIApiRouter>(m_veinEntry);

    m_router->setUpRoutes();
    QObject::connect(m_handler.get(), &OpenAPI::OAIApiRequestHandler::requestReceived, [&](QHttpEngine::Socket *socket) {
        m_router->processRequest(socket);
    });
    m_server.setHandler(m_handler.get());
}

bool HttpServerSetup::startListening(const QHostAddress &address, quint16 port)
{
    qDebug() << "Serving on " << address.toString() << ":" << port;
    // Attempt to get systemd socket
    qInfo("Return of sd_listen_fds: %i", sd_listen_fds(0));

    if(sd_listen_fds(0) == 1) {
        m_server.setSocketDescriptor(SD_LISTEN_FDS_START);
        qInfo("Using Systemd socket");
    }
    else {
        // Attempt to listen on the specified port
        if (!m_server.listen(address, port)) {
            qCritical("Unable to listen on the specified port.");
            return false;
        }
        qInfo("Using QTCPServer socket");
    }
    return true;
}
