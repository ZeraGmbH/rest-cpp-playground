#ifndef OAIANYTYPE_H
#define OAIANYTYPE_H

#include <QVariant>
#include <QJsonValue>

typedef QVariant OAIAnyType;

namespace OpenAPI {
    bool fromJsonValue(QVariant &value, const QJsonValue &raw);
    QJsonValue toJsonValue(const QVariant &value);

    QVariant jsonToVariant(const QString &json);
}

#endif // OAIANYTYPE_H
