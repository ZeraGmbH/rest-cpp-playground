#include <OAIAnyType.h>

#include <QJsonDocument>

namespace OpenAPI {

    QVariant jsonToVariant(const QString &json){
        // Wrap JSON serialized value in a dummy object do simplify parsing.
        QString docValue = QString("{\"d\":%1}").arg(json);

        // Parse the dummy object.
        QJsonDocument doc(QJsonDocument::fromJson(docValue.toUtf8()));

        // Copy the original value deserialized as a variant to the RPC actual parameter map.
        return doc["d"].toVariant();
    }

    bool fromJsonValue(QVariant &value, const QJsonValue &raw){
        QVariant cvt(raw.toVariant());

        if(cvt.type() == QVariant::Invalid) return false;

        value = cvt;

        return true;
    }

    QJsonValue toJsonValue(const QVariant &value){
        return value.toJsonValue();
    }
}
