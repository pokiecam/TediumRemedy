#include "stranger.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

Stranger::Stranger(QObject *parent) :
    QObject(parent)
{
    nam = new QNetworkAccessManager(this);
    QObject::connect(nam, SIGNAL(finished(QNetworkReply*)), this, SLOT(urlRequestFinished(QNetworkReply*)));
}

void Stranger::StartConversation() {
    EndConversation();

    QUrl requestUrl("http://front2.omegle.com/start?rcs=1&firstevents=1&spid=&randid=MG6PZ6ZP&lang=fr");
    const QNetworkRequest request(requestUrl);
    const QByteArray data;
    QNetworkReply *reply = nam->post(request, data);
}

void Stranger::EndConversation() {
    QUrl requestUrl("http://front2.omegle.com/disconnect");
    const QNetworkRequest request(requestUrl);
    const QByteArray data = QByteArray("id="+QUrl::toPercentEncoding(clientID));
    QNetworkReply *reply = nam->post(request, data);
}


void Stranger::SendMessage(QString &messageText) {
    QUrl requestUrl("http://front2.omegle.com/send");
    const QNetworkRequest request(requestUrl);
    const QByteArray data = QByteArray("msg=" + QUrl::toPercentEncoding(messageText) +"&id="+QUrl::toPercentEncoding(clientID));
    QNetworkReply *reply = nam->post(request, data);
}

void Stranger::StartTyping() {
    QUrl requestUrl("http://front2.omegle.com/typing");
    const QNetworkRequest request(requestUrl);
    const QByteArray data = QByteArray("id="+QUrl::toPercentEncoding(clientID));
    QNetworkReply *reply = nam->post(request, data);
}

void Stranger::StopTyping() {
    QUrl requestUrl("http://front2.omegle.com/stopTyping");
    const QNetworkRequest request(requestUrl);
    const QByteArray data = QByteArray("id="+QUrl::toPercentEncoding(clientID));
    QNetworkReply *reply = nam->post(request, data);
}

void Stranger::urlRequestFinished(QNetworkReply *reply) {
    QByteArray replyData = reply->readAll();
    delete reply;

    QString replyText(replyData);
    qDebug() << replyText;

    QJsonParseError parseError;
    QJsonDocument document = QJsonDocument::fromJson(replyData, &parseError);
    if(parseError.error != QJsonParseError::NoError)
        return; //error parsing json object


    if(document.isObject() && document.object().find("clientID")!=document.object().end()) {
        //we received a clientID
        clientID = document.object()["clientID"].toString();
        qDebug() << "Got client id: " << clientID;
        pollNewEvents();
    } else if(document.isArray() && !document.array().isEmpty()) {
        //first element of this array should be an array of [command, param1, ...]

        if(document.array()[0].isArray()) {
            QJsonArray commandWithArgs = document.array()[0].toArray();
            if(!commandWithArgs.isEmpty()) {
                if(processEvent(commandWithArgs))
                    pollNewEvents();
            }
        }
    }
}

void Stranger::pollNewEvents() {
    QUrl requestUrl("http://front2.omegle.com/events");
    const QNetworkRequest request(requestUrl);
    const QByteArray data = QByteArray("id=" + QUrl::toPercentEncoding(clientID));
    QNetworkReply *reply = nam->post(request, data);
}

bool Stranger::processEvent(QJsonArray &commandWithArgs) {
    QString eventName = commandWithArgs[0].toString();\
    if(eventName == "strangerDisconnected") {
        emit StrangerDisconnected();
        return false;
    } else if(eventName == "gotMessage") {
        const QString messageText = commandWithArgs[1].toString();
        emit ReceivedMessage(messageText);
    } else if(eventName == "typing") {
        emit StrangerStartsTyping();
    } else if(eventName == "stoppedTyping") {
        emit StrangerStopsTyping();
    } else if(eventName == "connected") {
        emit ConversationStarted();
    }
    return true;
}