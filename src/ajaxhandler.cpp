

#include <iostream>
#include <string>
#include <memory>

#include <QDebug>

#include "http_server/reply.hpp" // for stock reply HTTP 202 'accepted'
#include "http_server/request.hpp"

#include "ajaxhandler.h"


using namespace std;

unsigned int str2int(const string& str, int h = 0)
{
    return !str.c_str()[h] ? 5381 : (str2int(str.c_str(), h+1) * 33) ^ str.c_str()[h];
}

using namespace std;
using namespace http::server; // boost asio HTTP server

// taken from http://stackoverflow.com/questions/16388510/evaluate-a-string-with-a-switch-in-c
constexpr unsigned int str2int(const char* str, int h = 0)
{
        return !str[h] ? 5381 : (str2int(str, h+1)*33) ^ str[h];
}

void AjaxHandler::handle_request(const request& request, reply& response)
{
    // Decode url to path.
    string request_path;
    if (!url_decode(request.uri, request_path))
    {
        cerr << "Unable to decode URI: " << request_path << endl;
        response = reply::stock_reply(reply::bad_request);
        return;
    }

    // Request path must be absolute and not contain "..".
    if (request_path.empty() || request_path[0] != '/'
            || request_path.find("..") != string::npos)
    {
        cerr << "Invalid URI: " << request_path << endl;
        response = reply::stock_reply(reply::bad_request);
        return;
    }

    const string ANNOTATION("annotation=");

    if (request_path.find(ANNOTATION) != string::npos)
    {
        string query = request_path.substr(request_path.find(ANNOTATION) + ANNOTATION.size());
        bool parsingSuccessful = reader.parse( query, root );
        if (!parsingSuccessful) {
            cerr << "Invalid command: " << query << endl;
            response = reply::stock_reply(reply::bad_request);
            return;
        }
        response = process_annotation(root);
    }
    else if (request_path.find("state") != string::npos)
    {
        response = process_get_state();
    }
    else
    {
        cerr << "URI must contains a 'content': " << request_path << endl;
        response = reply::stock_reply(reply::bad_request);
        return;
    }



}

reply AjaxHandler::process_annotation(const Json::Value& msg)
{

    vector<StreamType> streams;
    auto type = AnnotationType::IRRELEVANT;

    switch(str2int(msg["stream"].asString())) {
    case str2int("global"):
        streams.push_back(StreamType::GLOBAL);
        break;
    case str2int("purple"):
        streams.push_back(StreamType::PURPLE);
        break;
    case str2int("yellow"):
        streams.push_back(StreamType::YELLOW);
        break;
    case str2int("both"):
        streams.push_back(StreamType::PURPLE);
        streams.push_back(StreamType::YELLOW);
        break;
    default:
        qDebug() << "Invalid stream type: " << QString::fromStdString(msg["stream"].asString());
        return reply::stock_reply(reply::bad_request);
    }

    switch(str2int(msg["type"].asString())) {
    case str2int("hostile"):
        type = AnnotationType::HOSTILE;
        break;
   case str2int("prosocial"):
        type = AnnotationType::PROSOCIAL;
        break;
    case str2int("assertive"):
        type = AnnotationType::ASSERTIVE;
        break;
    case str2int("passive"):
        type = AnnotationType::PASSIVE;
        break;
    case str2int("adultseeking"):
        type = AnnotationType::ADULTSEEKING;
        break;
    default:
        qDebug() << "Invalid annotation type: " << QString::fromStdString(msg["type"].asString());
        return reply::stock_reply(reply::bad_request);
    }

    for (auto s : streams) {
        emit annotationReceived(s, type);
    }

    qDebug() << "Annotation successfully created";
    return reply::json_reply("ok");
    //return reply::stock_reply(reply::accepted);
    //}
    //else
    //{
    //    cerr << "Invalid command: Unknown source " << id << endl;
    //    return reply::stock_reply(reply::bad_request);
    //}
}

reply AjaxHandler::process_get_state()
{
    //return reply::json_reply(table->sources_to_JSON());

    return reply::stock_reply(reply::accepted);
}
