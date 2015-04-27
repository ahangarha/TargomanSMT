/*************************************************************************
 * Copyright © 2012-2015, Targoman.com
 *
 * Published under the terms of TCRL(Targoman Community Research License)
 * You can find a copy of the license file with distributed source or
 * download it from http://targoman.com/License.txt
 *
 *************************************************************************/
/**
 @author S. Mohammad M. Ziabary <smm@ziabary.com>
 */

#include <QJsonObject>
#include <unistd.h>
#include <QtConcurrent/QtConcurrent>
#include "Private/clsConfigManagerOverNet.h"
#include "JSONConversationProtocol.h"

namespace Targoman {
namespace Common {
namespace Configuration {
namespace Private {

tmplConfigurable<int> clsConfigNetworkServer::ListenPort(
        "ConfigManager/AdminPort",
        "If set greater than zero Initializes a network channel to monitor and control application",
        0,
        [] (const intfConfigurable& _item, QString& _errorMessage){
            if (_item.toVariant().toUInt() > 60000){
                _errorMessage = "Invalid port to listen: " + _item.toVariant().toString();
                return false;
            }
            return true;
        },
        "",
        "PORT",
        "admin-port",
        (enuConfigSource::Type)(enuConfigSource::Arg | enuConfigSource::File),
        false
        );

tmplConfigurable<bool> clsConfigNetworkServer::AdminLocal(
        "ConfigManager/AdminLocal",
        "If set to true it will just listen to local connections.",
        false,
        [] (const intfConfigurable&, QString&){
            return true;
        },
        "",
        "",
        "admin-just-local",
        (enuConfigSource::Type)(enuConfigSource::Arg | enuConfigSource::File),
        false
        );

tmplConfigurable<bool> clsConfigNetworkServer::WaitPortReady(
        "ConfigManager/WaitPortReady",
        "If set to true it will wait till port is ready checking every 500ms.",
        false,
        [] (const intfConfigurable&, QString&){
            return true;
        },
        "",
        "",
        "admin-wait-port-ready",
        (enuConfigSource::Type)(enuConfigSource::Arg | enuConfigSource::File),
        false
        );

tmplConfigurable<int> clsConfigNetworkServer::MaxSessionTime(
        "ConfigManager/MaxSessiontime",
        "Max allowed time for a session. This is independent from idle time and must be greater. -1 means no limit",
        -1,
        [] (const intfConfigurable& _item, QString& _errorMessage){
            int MaxIdleTime = ConfigManager::instance().getConfig("ConfigManager/MaxIdleTime").toInt();
            if (_item.toVariant().toInt() >= 0 &&
                    (MaxIdleTime <0 ||
                     _item.toVariant().toInt() < MaxIdleTime)){
                _errorMessage = "Invalid Max Session Time. It must be greater than Max IdleTime";
                return false;
            }
            return true;
        },
        "",
        "SECONDS",
        "admin-max-session-time",
        (enuConfigSource::Type)(enuConfigSource::Arg | enuConfigSource::File),
        false
        );

tmplConfigurable<int> clsConfigNetworkServer::MaxIdleTime(
        "ConfigManager/MaxIdleTime",
        "Max allowed time for a session. This is independent from idle time and must be greater. -1 means no limit",
        -1,
        [] (const intfConfigurable& _item, QString& _errorMessage){
            int MaxSessionTime = ConfigManager::instance().getConfig("ConfigManager/MaxSessiontime").toInt();
            if (_item.toVariant().toInt() == 0 || (
                        _item.toVariant().toInt() > 0 &&
                        (MaxSessionTime > 0 ||
                         _item.toVariant().toInt() < MaxSessionTime))){
                _errorMessage = "Invalid Max Idle Time. It must be greater than zero but less than Max Session Time";
                return false;
            }
            return true;
        },
        "",
        "SECONDS",
        "admin-max-idle-time",
        (enuConfigSource::Type)(enuConfigSource::Arg | enuConfigSource::File),
        false
        );

tmplConfigurable<quint16> clsConfigNetworkServer::MaxConnections(
        "ConfigManager/MaxConnections",
        "Max administration connections allowed",
        1,
        [] (const intfConfigurable&, QString&){
            return true;
        },
        "",
        "MAX_ALLOWED",
        "max-connections",
        (enuConfigSource::Type)(enuConfigSource::Arg | enuConfigSource::File),
        false
        );

clsConfigNetworkServer::clsConfigNetworkServer(clsConfigManagerPrivate &_configManager) :
    ConfigManagerPrivate(_configManager),
    ActorUUID(_configManager.ActorUUID)
{}

clsConfigNetworkServer::~clsConfigNetworkServer()
{
    //Just to suppress compiler erro using QScopedPointer
}

void clsConfigNetworkServer::start(bool _justCheck)
{
    bool PrintMessage = true;
    if (this->ListenPort.value() > 0){
        do{
            if (!this->listen(this->AdminLocal.value() ? QHostAddress::LocalHost : QHostAddress::Any,
                                   this->ListenPort.value())){
                if (clsConfigNetworkServer::WaitPortReady.value() == false)
                    throw exConfigurationServer(QString("Unable to Start Server on: %1:%2").arg(
                                                    this->AdminLocal.value() ? "localhost" : "0.0.0.0").arg(
                                                    this->ListenPort.value()));
                else{
                    if (PrintMessage && _justCheck == false){
                        TargomanInfo(5, "Waiting for port %d ready...", this->ListenPort.value());
                        PrintMessage = false;
                    }
                    usleep(500000);
                }
            }else
                break;
        }while(_justCheck == false);
    }

    if (_justCheck)
        this->close();
    else
        TargomanInfo(5, QString("Configuration server has been started on %1:%2").arg(
                         this->AdminLocal.value() ? "localhost" : "0.0.0.0").arg(
                         this->ListenPort.value()))
}

bool clsConfigNetworkServer::check()
{
    if (this->ListenPort.value() > 0){
        this->start(true);
        return true;
    }else
        return true;
}

void clsConfigNetworkServer::incomingConnection(qintptr _socketDescriptor)
{
    clsClientThread* CLT = new clsClientThread(_socketDescriptor,
                                               this->ConfigManagerPrivate,
                                               this);

    connect(CLT, SIGNAL(finished()), CLT, SLOT(deleteLater()));

    CLT->start();
}

/******************************************************************************/

clsClientThread::clsClientThread(qintptr _socketDescriptor,
                                 clsConfigManagerPrivate &_configManager,
                                 QObject* _parent):
    QThread(_parent),
    SocketDescriptor(_socketDescriptor),
    ConfigManagerPrivate(_configManager),
    ActorUUID(_configManager.ActorUUID)
{
    this->AllowedToChange = false;
    this->AllowedToView   = false;
}

void clsClientThread::slotReadyRead()
{
    try
    {
        if (this->Socket->bytesAvailable() < 3)
            return;

        if (this->Socket->bytesAvailable() > 20000)
            this->sendError(enuReturnType::InvalidStream,"Stream Data is too long");

        QByteArray ReceivedBytes =this->Socket->readLine().trimmed();
        TargomanDebug(9,"Received: "+  ReceivedBytes);

        if(ReceivedBytes.isEmpty())
            return;

        JSONConversationProtocol::stuRequest Request =
                JSONConversationProtocol::parseRequest(ReceivedBytes);
        try{
            if (Request.Name == "simplePing"){
                JSONConversationProtocol::stuPong Pong;
                emit this->ConfigManagerPrivate.Parent.sigPing(Pong);
                return this->sendResult(JSONConversationProtocol::preparePong(Pong));
            }
            /************************************************************/
            if (Request.Name == "login"){
                QString Pass;
                if (Request.Args.contains("l")){
                    this->ActorName = Request.Args.value("l").toString();
                    Pass =      Request.Args.value("p").toString();
                }else if (Request.Args.contains("ssid"))
                    this->ActorName = "$SSID$" + Request.Args.value("ssid").toString();

                if (this->ActorName.isEmpty())
                    this->ActorName = "UNKNOWN";
                emit this->ConfigManagerPrivate.Parent.sigValidateAgent(
                            this->ActorName,
                            Pass,
                            this->Socket->peerAddress().toString(),
                            this->AllowedToView,
                            this->AllowedToChange);

                if (this->AllowedToView) {
                    TargomanLogInfo(5, QString("User: %1 Logged in with %2 Access").arg(
                                        this->ActorName).arg(
                                        this->AllowedToChange ? "ReadWrite" : "ReadOnly"));

                    if (this->AllowedToChange)
                        return this->sendResult(JSONConversationProtocol::prepareResult(
                                              Request.CallBack,
                                              Request.CallString,
                                              3));
                    else
                        return this->sendResult(JSONConversationProtocol::prepareResult(
                                              Request.CallBack,
                                              Request.CallString,
                                              1));
                } else if (this->ActorName.isEmpty()) {
                    TargomanLogWarn(6, "Attemp to login from"<<
                                    this->Socket->peerAddress().toString()<<":"<<
                                    this->Socket->peerPort()<<" Failed");
                    return this->sendError(enuReturnType::InvalidLogin,
                                           "Invalid User/Password");
                } else {
                    TargomanLogWarn(6,
                                    QString("User: %1 attemped to Login but not enough access").arg(
                                        this->ActorName));
                    return this->sendError(enuReturnType::InvalidLogin,
                                           QString("Not enough access"));
                }
            }
            /************************************************************/
            if (this->AllowedToView == false)
                return this->sendError(enuReturnType::InvalidLogin,
                                       QString("Invalid Request. Please login first"));
            /************************************************************/
            if (Request.Name == "walk")
            {
                bool ShowDetails =  Request.Args.value("dt", false).toBool();

                QVariantList TableRows;
                QVariantList Table;

                if (ShowDetails)
                    TableRows.insert(
                                TableRows.end(),
                                QVariantList()<<"Path"<<"Value"<<"Type"<<"Updatable"<<"Options");
                else
                    TableRows.insert(
                                TableRows.end(),
                                QVariantList()<<"Path"<<"Value");

                Table.append(TableRows);

                foreach(const intfConfigurable* Item, this->ConfigManagerPrivate.Configs.values()){
                    TableRows.clear();
                    if (Item->remoteView() == false)
                        continue;

                    if (ShowDetails)
                        TableRows.insert(TableRows.end(),
                                         QVariantList()<<
                                         Item->configPath()<<
                                         Item->toVariant().toString()<<
                                         Item->typeString()<<
                                         (Item->canBeConfigured(enuConfigSource::Net) &&
                                          this->AllowedToChange)<<
                                         Item->validValues()
                                         );
                    else
                        TableRows.insert(TableRows.end(),
                                         QVariantList()<<
                                         Item->configPath()<<
                                         Item->toVariant().toString()
                                         );

                    Table.append(TableRows);
                }
                QVariantMap ReturnVals;
                ReturnVals.insert("t", Table);
                return this->sendResult(JSONConversationProtocol::prepareResult(
                                      Request.CallBack,
                                      Request.CallString,
                                      Table.length(),
                                      ReturnVals));
            }
            /************************************************************/
            if (Request.Name == "query") {
                QString ObjectPath =  Request.Args.value("p").toString();
                intfConfigurable* ConfigItem = this->ConfigManagerPrivate.Configs.value(ObjectPath, NULL);
                if (ConfigItem)
                    return this->sendResult(JSONConversationProtocol::prepareResult(
                                          Request.CallBack,
                                          Request.CallString,
                                          ConfigItem->toVariant().toString()));
                else
                    return this->sendResult(JSONConversationProtocol::prepareError(
                                          Request.CallBack,
                                          Request.CallString,
                                          enuReturnType::ObjectNotFound,
                                          ObjectPath));
            }
            /************************************************************/
            if (Request.Name == "bulkQuery"){
                QString ParenPath       = Request.Args.value("p").toString();
                bool    IsRegex         = Request.Args.value("rx",false).toBool();
                bool    ShowDetails     = Request.Args.value("dt",false).toBool();
                qint8   JustUpdatable   = Request.Args.value("up", -2).toInt();
                QString JustType        = Request.Args.value("tp").toString();
                QString StripString     = Request.Args.value("ss").toString();

                if (ParenPath.isEmpty())
                    return this->sendResult(JSONConversationProtocol::prepareError(
                                          Request.CallBack,
                                          Request.CallString,
                                          enuReturnType::ObjectNotFound,
                                          ParenPath));

                if (ParenPath.endsWith('/') == false)
                    ParenPath += IsRegex ? "(\\/|$)" : "/";

                ParenPath.replace(QRegExp("//*"),"/");

                QList<intfConfigurable*> ConfigItems =
                        this->ConfigManagerPrivate.configItems(ParenPath, IsRegex, false);

                if (ConfigItems.isEmpty())
                    return this->sendResult(JSONConversationProtocol::prepareError(
                                          Request.CallBack,
                                          Request.CallString,
                                          enuReturnType::ObjectNotFound,
                                          ParenPath));

                QVariantList TableRows;
                QVariantList Table;

                if (ShowDetails)
                    TableRows.insert(
                                TableRows.end(),
                                QVariantList()<<"Path"<<"Value"<<"Type"<<"Updatable"<<"Options");
                else
                    TableRows.insert(
                                TableRows.end(),
                                QVariantList()<<"Path"<<"Value");

                Table.append(TableRows);

                foreach(intfConfigurable* Item, ConfigItems) {
                    if (Item->remoteView() == false
                            || (JustUpdatable != -2
                                && (Item->canBeConfigured(enuConfigSource::Net) &&
                                    this->AllowedToChange) != JustUpdatable)
                            || (JustType.size() &&
                                JustType != Item->typeString()))
                        continue;

                    TableRows.clear();

                    QString Path = Item->configPath();
                    if (Path.startsWith(StripString))
                        Path = Path.mid(StripString.size());

                    if (ShowDetails)
                        TableRows.insert(TableRows.end(),
                                         QVariantList()<<
                                         Item->configPath()<<
                                         Item->toVariant().toString()<<
                                         Item->typeString()<<
                                         (Item->canBeConfigured(enuConfigSource::Net) &&
                                          this->AllowedToChange)<<
                                         Item->validValues()
                                         );
                    else
                        TableRows.insert(TableRows.end(),
                                         QVariantList()<<
                                         Item->configPath()<<
                                         Item->toVariant().toString()
                                         );
                    Table.append(TableRows);
                }
                QVariantMap ReturnVals;
                ReturnVals.insert("t", Table);
                return this->sendResult(JSONConversationProtocol::prepareResult(
                                      Request.CallBack,
                                      Request.CallString,
                                      Table.length(),
                                      ReturnVals));
            } //if (Request.Name == "bulkQuery")

            /************************************************************/
            if (Request.Name == "set") {
                if (this->AllowedToChange == false)
                    return this->sendResult(JSONConversationProtocol::prepareError(
                                          Request.CallBack,
                                          Request.CallString,
                                          enuReturnType::InvalidAction,
                                          "Not Allowed"));

                QString ObjectPath       = Request.Args.value("p").toString();
                if (ObjectPath.isEmpty())
                    return this->sendResult(JSONConversationProtocol::prepareError(
                                          Request.CallBack,
                                          Request.CallString,
                                          enuReturnType::ObjectNotFound,
                                          ObjectPath));

                QVariant NewValue = Request.Args.value("vl");
                if (NewValue.isNull() == false) {
                    intfConfigurable* ConfigItem =
                            this->ConfigManagerPrivate.Configs.value(ObjectPath, NULL);
                    if (ConfigItem){
                        if (ConfigItem->remoteView() == false ||
                                ConfigItem->canBeConfigured(enuConfigSource::Net) == false)
                            return this->sendResult(JSONConversationProtocol::prepareError(
                                                  Request.CallBack,
                                                  Request.CallString,
                                                  enuReturnType::InvalidUpdateSource,
                                                  ObjectPath));

                        QString ErrorMessage;
                        QVariant OldValue = ConfigItem->toVariant();
                        try{
                            ConfigItem->setFromVariant(NewValue);
                            if (ConfigItem->crossValidate(ErrorMessage) == false){
                                ConfigItem->setFromVariant(OldValue);
                                return this->sendResult(JSONConversationProtocol::prepareError(
                                                      Request.CallBack,
                                                      Request.CallString,
                                                      enuReturnType::InvalidData,
                                                      ErrorMessage + " On: " + ObjectPath));
                            }else{
                                TargomanLogInfo(5, QString("User: %1 Changed value of %2 to %4").arg(
                                                    this->ActorName).arg(
                                                    ObjectPath).arg(
                                                    NewValue.toString()));
                                return this->sendResult(
                                                  JSONConversationProtocol::prepareResult(
                                                      Request.CallBack,
                                                      Request.CallString,
                                                      NewValue.toString()));
                            }//else
                        }catch(exConfiguration &e){
                            return this->sendResult(JSONConversationProtocol::prepareError(
                                                  Request.CallBack,
                                                  Request.CallString,
                                                  enuReturnType::InvalidData,
                                                  e.what().split(">;ex").first() + " On: " + ObjectPath));
                        }//catch
                    }else
                        return this->sendResult(JSONConversationProtocol::prepareError(
                                              Request.CallBack,
                                              Request.CallString,
                                              enuReturnType::ObjectNotFound,
                                              ObjectPath));
                } else
                    this->sendResult(JSONConversationProtocol::prepareError(
                                   Request.CallBack,
                                   Request.CallString,
                                   enuReturnType::InvalidAction,
                                   "Unimplemented Set to NULL Command on: " + ObjectPath));
            }// if (Request.Name == "set")

            /************************************************************/
            if (Request.Name == "rpc") {
                QString ProcedureName = Request.Args.value("rpn").toString();

                if (ProcedureName.isEmpty())
                    return this->sendResult(JSONConversationProtocol::prepareError(
                                          Request.CallBack,
                                          Request.CallString,
                                          enuReturnType::ObjectNotFound,
                                          "Invalid RPC call without procedure name"));

                try{
                    QVariant Return;
                    emit this->ConfigManagerPrivate.Parent.sigRPC(ProcedureName, Request.Args, Return);

                    if (Return.isValid() == false){
                        this->sendResult(JSONConversationProtocol::prepareError(
                                       Request.CallBack,
                                       Request.CallString,
                                       enuReturnType::ObjectNotFound,
                                       "Procedure: " + ProcedureName +" Not Found"));
                        return;
                    }

                    QVariantList TableRows;
                    QVariantList Table;

                    TableRows.insert(
                                TableRows.end(),
                                QVariantList()<<"ArgName"<<"ArgValue");

                    Table.append(TableRows);

                    QVariantMap::iterator ArgumentsIter = Request.Args.begin();
                    while (ArgumentsIter != Request.Args.end())
                    {
                        TableRows.clear();
                        TableRows.insert(TableRows.end(),
                                         QVariantList()<<
                                         ArgumentsIter.key()<<
                                         ArgumentsIter.value());
                        Table.append(TableRows);

                        ArgumentsIter++;
                    }

                    QVariantMap ReturnVals;
                    ReturnVals.insert("t", Table);
                    return this->sendResult(JSONConversationProtocol::prepareResult(
                                          Request.CallBack,
                                          Request.CallString,
                                          Return.toString(),
                                          ReturnVals));

                }catch(exTargomanBase &e){
                    return this->sendResult(JSONConversationProtocol::prepareError(
                                          Request.CallBack,
                                          Request.CallString,
                                          enuReturnType::InvalidData,
                                          e.what()));
                }
            }//if (Request.Name == "rpc")

            /************************************************************/
            //Finally seems that request name is invalid
            return this->sendResult(JSONConversationProtocol::prepareError(
                                  Request.CallBack,
                                  Request.CallString,
                                  enuReturnType::InvalidAction,
                                  "Invalid request " + Request.Name));
        } catch (exTargomanBase &e) {
            this->sendResult(JSONConversationProtocol::prepareError(
                           Request.CallBack,
                           Request.CallString,
                           enuReturnType::Undefined,
                           e.what()));
        }
    }catch(exTargomanBase &e){
        this->sendError(enuReturnType::InvalidStream,e.what());
    }
}

void clsClientThread::slotDisconnected()
{
    TargomanLogInfo(5, QString("Client with id=%3 disconnected").arg(
                        this->SocketDescriptor));
    this->Socket->deleteLater();
    exit(0);
}

void clsClientThread::run()
{
    this->Socket = new QTcpSocket;
    if(!this->Socket->setSocketDescriptor(this->SocketDescriptor)){
        emit error(this->Socket->error());
        return;
    }


    connect(this->Socket, SIGNAL(readyRead()),
            this,           SLOT(slotReadyRead()), Qt::DirectConnection);
    connect(this->Socket, SIGNAL(disconnected()), this, SLOT(slotDisconnected()));

    TargomanLogInfo(5, QString("New Configuration Admin from %1:%2 with id=%3 connected").arg(
                        this->Socket->peerAddress().toString()).arg(
                        this->Socket->peerPort()).arg(this->SocketDescriptor));

    this->exec();
}

void clsClientThread::sendError(enuReturnType::Type _type, const QString& _message)
{
    QString Message =
            JSONConversationProtocol::prepareError("", "", _type, _message);
    this->Socket->write(Message.toUtf8());

    TargomanDebug(8,"Sent and disconnected ("<<
                  this->ActorName<<"@"<<
                  this->Socket->peerAddress().toString()<<":"<<
                  this->Socket->peerPort()<<"): "<<Message);
    this->Socket->disconnectFromHost();
    this->Socket->waitForDisconnected();
}

void clsClientThread::sendResult(const QString &_data)
{
    this->Socket->write(_data.toUtf8());
    TargomanDebug(8, "Sent to Client ("<<
                  this->ActorName<<"@"<<
                  this->Socket->peerAddress().toString()<<":"<<
                  this->Socket->peerPort()<<"): "<<_data);
}

}
}
}
}
