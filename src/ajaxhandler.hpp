#ifndef AJAXHANDLER_H
#define AJAXHANDLER_H

#include <memory>
#include <json/json.h>
#include <QObject>

#include "annotation.hpp"
#include "http_server/request_handler.hpp"

class AjaxHandler : public QObject, public http::server::request_handler
{
    Q_OBJECT

public:

    AjaxHandler(QObject * parent = nullptr):QObject(parent), http::server::request_handler("./html"), paused_(false) {}

    virtual void handle_request(const http::server::request& request,
                                http::server::reply& response) override;

    Q_SIGNAL void annotationReceived(StreamType stream, AnnotationType annotation);
    Q_SIGNAL void jumpBy(int secs);
    Q_SIGNAL void jumpTo(int secs);
    Q_SIGNAL void pause();
    Q_SIGNAL void resume();

    Q_SLOT void paused();
    Q_SLOT void resumed() {paused_=false;}

private:

    http::server::reply process_annotation(const Json::Value& msg);
    http::server::reply process_get_state();

    Json::Value root; // will contains the root value after parsing.
    Json::Reader reader;

    bool paused_;


};
#endif // AJAXHANDLER_H
