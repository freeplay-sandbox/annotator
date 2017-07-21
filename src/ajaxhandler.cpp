

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <memory>

#include <QDebug>

#include "http_server/mime_types.hpp"
#include "http_server/reply.hpp"
#include "http_server/request.hpp"

#include "ajaxhandler.hpp"


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
    const string ACTIVEANNOTATIONS("activeannotations");
    const string PAUSE("pause");
    const string ISPAUSED("ispaused");
    const string RESUME("resume");
    const string JUMPBY("jumpby=");
    const string JUMPTO("jumpto=");
    const string CLEARALL("clearall");

    //cout << request_path << endl;
    if (request_path.find(ANNOTATION) != string::npos)
    {
        string query = request_path.substr(request_path.find(ANNOTATION) + ANNOTATION.size());
        bool parsingSuccessful = reader.parse( query, root );
        if (!parsingSuccessful) {
            cerr << "Invalid annotation: " << query << endl;
            response = reply::stock_reply(reply::bad_request);
            return;
        }
        response = process_annotation(root);
        return;
    }
    else if (request_path.find(ACTIVEANNOTATIONS) != string::npos)
    {
        response = reply::stock_reply(reply::bad_request);
        return;
    }
    else if (request_path.find(ISPAUSED) != string::npos)
    {
        response = reply::json_reply(paused_ ? true : false);
        return;
    }
    else if (request_path.find(PAUSE) != string::npos)
    {
        emit pause();
        response = reply::json_reply("true");
        return;
    }
    else if (request_path.find(RESUME) != string::npos)
    {
        emit resume();
        response = reply::json_reply("true");
        return;
    }
    else if (request_path.find(JUMPBY) != string::npos)
    {
        string time = request_path.substr(request_path.find(JUMPBY) + JUMPBY.size());
        emit jumpBy(stoi(time));
        response = reply::json_reply("true");
        return;
   }
    else if (request_path.find(JUMPTO) != string::npos)
    {
        string time = request_path.substr(request_path.find(JUMPTO) + JUMPTO.size());
        emit jumpTo(stoi(time));
        response = reply::json_reply("true");
        return;
  }
    else if (request_path.find(CLEARALL) != string::npos)
    {
        emit clearAllAnnotations();
        response = reply::json_reply("true");
        return;
  }

    // not processing an AJAX request -> then serve files!
    else {


        // If path ends in slash (i.e. is a directory) then add "index.html".
        if (request_path[request_path.size() - 1] == '/')
        {
            request_path += "index.html";
        }

        // Determine the file extension.
        std::size_t last_slash_pos = request_path.find_last_of("/");
        std::size_t last_dot_pos = request_path.find_last_of(".");
        std::string extension;
        if (last_dot_pos != std::string::npos && last_dot_pos > last_slash_pos)
        {
            extension = request_path.substr(last_dot_pos + 1);
        }

        // Open the file to send back.
        std::string full_path = doc_root_ + request_path;
        std::ifstream is(full_path.c_str(), std::ios::in | std::ios::binary);
        if (!is)
        {
            response = reply::stock_reply(reply::not_found);
            return;
        }

        // Fill out the reply to be sent to the client.
        response.status = reply::ok;
        char buf[512];
        while (is.read(buf, sizeof(buf)).gcount() > 0)
            response.content.append(buf, is.gcount());
        response.headers.resize(2);
        response.headers[0].name = "Content-Length";
        response.headers[0].value = std::to_string(response.content.size());
        response.headers[1].name = "Content-Type";
        response.headers[1].value = mime_types::extension_to_type(extension);

    }

}

void AjaxHandler::paused() {cout << "paused!" << endl;paused_=true;}

reply AjaxHandler::process_annotation(const Json::Value& msg)
{

    vector<StreamType> streams;

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

    auto type = annotationFromName(msg["type"].asString());

    for (auto s : streams) {
        emit annotationReceived(s, type);
    }

    //qDebug() << "Annotation successfully created";
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
