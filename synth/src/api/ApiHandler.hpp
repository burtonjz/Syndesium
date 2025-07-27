#ifndef __API_HANDLER_HPP_
#define __API_HANDLER_HPP_

#include <nlohmann/json.hpp>

using json = nlohmann::json ;

// forward declarations
class Engine ;

class ApiHandler {
public:
    ApiHandler() = delete ;

    static void start(Engine* engine);
    static void onClientConnection(Engine* engine, int clientSock);
    static void handleClientMessage(Engine* engine, int clientSock, std::string jsonStr);
    static void sendApiResponse(int clientSock, json response);
};


#endif // __API_HANDLER_HPP_
