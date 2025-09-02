#ifndef __API_HANDLER_HPP_
#define __API_HANDLER_HPP_

#include <nlohmann/json.hpp>
#include "types/SocketType.hpp"

using json = nlohmann::json ;

// forward declarations
class Engine ;

// useful structs for API JSON parsing
struct ConnectionRequest {
    SocketType inboundSocket ;
    SocketType outboundSocket ;
    std::optional<int> inboundID ;
    std::optional<int> outboundID ;
    std::optional<bool> inboundIsModule ;
    std::optional<bool> outboundIsModule ;
};

class ApiHandler {
public:
    ApiHandler() = delete ;

    static void start(Engine* engine);
    static void onClientConnection(Engine* engine, int clientSock);
    static void handleClientMessage(Engine* engine, int clientSock, std::string jsonStr);
    static void sendApiResponse(int clientSock, json response);

private:
    // cable connection functions
    static ConnectionRequest parseConnectionRequest(json request);
    static bool routeConnectionRequest(Engine* engine, ConnectionRequest request);
    static bool handleSignalConnection(Engine* engine, ConnectionRequest request);
    static bool handleMidiConnection(Engine* engine, ConnectionRequest request);
    static bool handleModulationConnection(Engine* engine, ConnectionRequest request);
};


#endif // __API_HANDLER_HPP_
