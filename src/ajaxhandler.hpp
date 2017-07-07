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

    AjaxHandler(QObject * parent = nullptr):QObject(parent), http::server::request_handler("./html") {}

    virtual void handle_request(const http::server::request& request,
                                http::server::reply& response) override;

    Q_SIGNAL void annotationReceived(StreamType stream, AnnotationType annotation);

private:

    http::server::reply process_annotation(const Json::Value& msg);
    http::server::reply process_get_state();

    Json::Value root; // will contains the root value after parsing.
    Json::Reader reader;


};
#endif // AJAXHANDLER_H
