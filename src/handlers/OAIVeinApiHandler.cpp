/**
 * SourceApi
 * A Web API for controlling a source.
 *
 * The version of the OpenAPI document: v1
 *
 * NOTE: This class is auto generated by OpenAPI Generator (https://openapi-generator.tech).
 * https://openapi-generator.tech
 * Do not edit the class manually.
 */

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariantMap>
#include <QDebug>

#include "OAIVeinApiHandler.h"
#include "OAIVeinApiRequest.h"
#include "veinentrysingleton.h"

namespace OpenAPI {

OAIVeinApiHandler::OAIVeinApiHandler(){

}

OAIVeinApiHandler::~OAIVeinApiHandler(){

}

QList<OAIVeinGetResponse> OAIVeinApiHandler::generateBulkAnswer(QList<OAIVeinGetRequest> oai_vein_get_request)
{
    QList<OAIVeinGetResponse> response;
    for (const auto &item : oai_vein_get_request)
    {
        int entityId = item.getEntityId();
        QString componentName = item.getComponentName();
        OAIVeinGetResponse responseEntry;
        VeinStorage::AbstractDatabase* storage = VeinEntrySingleton::getInstance().getStorageDb();

        responseEntry.setComponentName(componentName);
        responseEntry.setEntityId(entityId);

        if(storage->hasStoredValue(item.getEntityId(), item.getComponentName())){
            QVariant value = storage->getStoredValue(item.getEntityId(), item.getComponentName());
            QString returnValue = variantToJsonString(value);
            responseEntry.setType(value.typeName());
            if(returnValue != "null")
            {
                responseEntry.setReturnInformation(returnValue);
                responseEntry.setStatus(200);
            }
            else
            {
                responseEntry.setReturnInformation("null");
                responseEntry.setStatus(422);
                responseEntry.setType("Invalid");
            }

        }
        else
        {
            responseEntry.setStatus(422);
            responseEntry.setType("Invalid");
            responseEntry.setReturnInformation("\"Timeout or not existing entity or component\"");
        }

        response.append(responseEntry);
    }
    return response;
}

QString OAIVeinApiHandler::variantToJsonString(QVariant input)
{
    QString typeName = input.typeName();
    QString returnValue;

    if (static_cast<QMetaType::Type>(input.type()) == QMetaType::QJsonObject) {
        QJsonObject jsonObj = QJsonValue::fromVariant(input).toObject();
        QJsonDocument doc = QJsonDocument(jsonObj);
        returnValue = doc.toJson(QJsonDocument::Compact);
    }
    else if(typeName.contains("List"))
    {
        QVariantList list = input.value<QVariantList>();
        QJsonArray jsonArray;
        for (const QVariant& item : list)
        {
            jsonArray.append(QJsonValue::fromVariant(item));
        }
        QJsonDocument doc = QJsonDocument(jsonArray);
        returnValue = doc.toJson(QJsonDocument::Compact);
    }
    else {
        QJsonValue jsonValue = QJsonValue::fromVariant(input);
        QJsonDocument doc = QJsonDocument(QJsonArray{jsonValue});
        QString serialized = doc.toJson(QJsonDocument::Compact);
        returnValue = serialized.mid(1, serialized.length() - 2);
    }

    return returnValue;
}


void OAIVeinApiHandler::apiV1VeinGet(qint32 entity_id, QString component_name) {
    Q_UNUSED(entity_id);
    Q_UNUSED(component_name);
    auto reqObj = qobject_cast<OAIVeinApiRequest*>(sender());
    if( reqObj != nullptr )
    {
        TaskSimpleVeinGetterPtr task = VeinEntrySingleton::getInstance().getFromVein(entity_id, component_name);
        std::shared_ptr<TaskSimpleVeinGetter> taskSharedPtr = std::move(task);

        auto conn = std::make_shared<QMetaObject::Connection>();
        *conn = connect(taskSharedPtr.get(), &TaskTemplate::sigFinish, this, [this ,reqObj, taskSharedPtr, conn, entity_id, component_name](bool ok, int taskId){

            OAIVeinGetResponse res;
            if (ok)
            {
                QVariant ret = taskSharedPtr->getValue();
                QString retString = variantToJsonString(ret);
                if(retString != "null")
                    res.setStatus(200);
                else
                    res.setStatus(422);

                res.setReturnInformation(retString);
                res.setType(taskSharedPtr->getValue().typeName());
                res.setComponentName(component_name);
                res.setEntityId(entity_id);

            }
            else
            {
                res.setReturnInformation("\"Timeout or not existing entity or component\"");
                res.setType("Invalid");
                res.setStatus(422);
            }

            reqObj->apiV1VeinGetResponse(res);
            disconnect(*conn);
        });
        taskSharedPtr->start();
    }
}

void OAIVeinApiHandler::apiV1VeinPost(QList<OAIVeinGetRequest> oai_vein_get_request) {
    Q_UNUSED(oai_vein_get_request);
    auto reqObj = qobject_cast<OAIVeinApiRequest*>(sender());
    if( reqObj != nullptr )
    {
        QList<OAIVeinGetResponse> res = generateBulkAnswer(oai_vein_get_request);

        reqObj->apiV1VeinPostResponse(res);
    }
}

void OAIVeinApiHandler::apiV1VeinPut(OAIVeinSet oai_vein_set) {
    auto reqObj = qobject_cast<OAIVeinApiRequest*>(sender());
    if( reqObj != nullptr )
    {
        TaskSimpleVeinSetterPtr task = VeinEntrySingleton::getInstance().setToVein(oai_vein_set.getEntityId(),oai_vein_set.getComponentName(), oai_vein_set.getNewValue());
        std::shared_ptr<TaskSimpleVeinSetter> taskSharedPtr = std::move(task);

        if (oai_vein_set.getEntityId() == 0)
        {
            OAIProblemDetails res;
            res.setStatus(500);
            res.setDetail("Not allowed: System entity is write protected");
            res.setTitle("Setter command output");
            res.setType("");
            reqObj->apiV1VeinPutResponse(res);
            return;
        }
        else if (!oai_vein_set.is_entity_id_Valid() || !oai_vein_set.is_component_name_Valid() || !oai_vein_set.is_new_value_Valid())
        {
            OAIProblemDetails res;
            res.setStatus(400);
            res.setDetail("Input not valid: Entity Id or Component name");
            res.setTitle("Setter command output");
            res.setType("");
            reqObj->apiV1VeinPutResponse(res);
            return;
        }

        auto conn = std::make_shared<QMetaObject::Connection>();
        *conn = connect(taskSharedPtr.get(), &TaskTemplate::sigFinish, this, [conn, reqObj, taskSharedPtr, oai_vein_set](bool ok, int taskId){
            OAIProblemDetails res;
            if (ok)
                res.setStatus(200);
            else
                res.setStatus(422);

            QString str = "Entity Id: " + QString::number(oai_vein_set.getEntityId()) + " Component name: " + oai_vein_set.getComponentName() + " New Value: " + oai_vein_set.getNewValue();
            res.setDetail(str);
            res.setTitle("Setter command output");
            res.setType("");
            reqObj->apiV1VeinPutResponse(res);
            disconnect(*conn);
        });
        taskSharedPtr->start();
    }
}
}
