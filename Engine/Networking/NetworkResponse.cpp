// ----------------------------------- // 
#ifndef LEVIATHAN_NETWORKRESPONSE
#include "NetworkResponse.h"
#endif
#include "Exceptions.h"
#include "Common/DataStoring/NamedVars.h"
#include "GameSpecificPacketHandler.h"
#include "NetworkedInput.h"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //
DLLEXPORT NetworkResponse::NetworkResponse(int inresponseto,
    PACKET_TIMEOUT_STYLE timeout, int timeoutvalue)
    : ResponseID(inresponseto),
    TimeOutValue(timeoutvalue), TimeOutStyle(timeout), ResponseType(NETWORKRESPONSETYPE_NONE)
{

}

DLLEXPORT NetworkResponse::NetworkResponse(sf::Packet &receivedresponse) :
    TimeOutValue(-1)
{
	// First thing is the response ID //
	receivedresponse >> ResponseID;

	// Second is the type, based on which we handle the rest of the data //
	int tmpval;
    receivedresponse >> tmpval;

    if(!receivedresponse)
        throw InvalidArgument("packet has invalid format");
    
	ResponseType = static_cast<NETWORKRESPONSETYPE>(tmpval);

	// Process based on the type //
	switch(ResponseType){
    case NETWORKRESPONSETYPE_NONE: case NETWORKRESPONSETYPE_KEEPALIVE:
    case NETWORKRESPONSETYPE_CLOSECONNECTION:
    case NETWORKRESPONSETYPE_REMOTECONSOLEOPENED:
    case NETWORKRESPONSETYPE_REMOTECONSOLECLOSED:
    case NETWORKRESPONSETYPE_SERVERHEARTBEAT: case NETWORKRESPONSETYPE_STARTHEARTBEATS:
		{
			// There is no data in the packet //
			ResponseData = NULL;
			return;
		}
    case NETWORKRESPONSETYPE_IDENTIFICATIONSTRINGS:
		{
			ResponseData = new NetworkResponseDataForIdentificationString(receivedresponse);
		}
		break;
    case NETWORKRESPONSETYPE_INVALIDREQUEST:
		{
			ResponseData = new NetworkResponseDataForInvalidRequest(receivedresponse);
		}
		break;
    case NETWORKRESPONSETYPE_SERVERSTATUS:
		{
			ResponseData = new NetworkResponseDataForServerStatus(receivedresponse);
		}
		break;
    case NETWORKRESPONSETYPE_SERVERALLOW:
		{
			ResponseData = new NetworkResponseDataForServerAllow(receivedresponse);
		}
		break;
    case NETWORKRESPONSETYPE_SERVERDISALLOW:
		{
			ResponseData = new NetworkResponseDataForServerDisallow(receivedresponse);
		}
		break;
    case NETWORKRESPONSETYPE_SYNCVALDATA:
		{
			ResponseData = new NetworkResponseDataForSyncValData(receivedresponse);
		}
		break;
    case NETWORKRESPONSETYPE_SYNCDATAEND:
		{
			ResponseData = new NetworkResponseDataForSyncDataEnd(receivedresponse);
		}
		break;
    case NETWORKRESPONSETYPE_CUSTOM:
		{
			ResponseData = new NetworkResponseDataForCustom(receivedresponse);
		}
		break;
    case NETWORKRESPONSETYPE_SYNCRESOURCEDATA:
		{
			ResponseData = new NetworkResponseDataForSyncResourceData(receivedresponse);
		}
		break;
    case NETWORKRESPONSETYPE_CREATENETWORKEDINPUT:
		{
			ResponseData = new NetworkResponseDataForCreateNetworkedInput(receivedresponse);
		}
		break;
    case NETWORKRESPONSETYPE_UPDATENETWORKEDINPUT:
		{
			ResponseData = new NetworkResponseDataForUpdateNetworkedInput(receivedresponse);
		}
		break;
    case NETWORKRESPONSETYPE_INITIAL_ENTITY:
        {
            ResponseData = new NetworkResponseDataForInitialEntity(receivedresponse);
        }
        break;
    case NETWORKRESPONSETYPE_ENTITY_CONSTRAINT:
        {
            ResponseData = new NetworkResponseDataForEntityConstraint(receivedresponse);
        }
        break;
    case NETWORKRESPONSETYPE_WORLD_FROZEN:
        {
            ResponseData = new NetworkResponseDataForWorldFrozen(receivedresponse);
        }
        break;
    case NETWORKRESPONSETYPE_ENTITY_UPDATE:
        {
            ResponseData = new NetworkResponseDataForEntityUpdate(receivedresponse);
        }
        break;
    case NETWORKRESPONSETYPE_ENTITY_DESTRUCTION:
        {
            ResponseData = new NetworkResponseDataForEntityDestruction(receivedresponse);
        }
        break;
    case NETWORKRESPONSETYPE_AI_CACHE_UPDATED:
        {
            ResponseData = new NetworkResponseDataForAICacheUpdated(receivedresponse);
        }
        break;
    case NETWORKRESPONSETYPE_AI_CACHE_REMOVED:
        {
            ResponseData = new NetworkResponseDataForAICacheRemoved(receivedresponse);
        }
        break;
    case NETWORKRESPONSETYPE_DISCONNECTINPUT:
        {
            ResponseData = new NetworkResponseDataForDisconnectInput(receivedresponse);
        }
        break;
    default:
		{
            Logger::Get()->Warning("NetworkResponse: unused type: "+
                Convert::ToString(ResponseType));
            throw InvalidArgument("packet has invalid format invalid response type");
		}
		break;
	}
}

DLLEXPORT NetworkResponse::~NetworkResponse(){
	SAFE_DELETE(ResponseData);
}
// ------------------------------------ //
DLLEXPORT void NetworkResponse::GenerateIdentificationStringResponse(
    NetworkResponseDataForIdentificationString* newddata)
{
	ResponseType = NETWORKRESPONSETYPE_IDENTIFICATIONSTRINGS;
	// Destroy old data if any //
	SAFE_DELETE(ResponseData);

	ResponseData = newddata;
}

DLLEXPORT void NetworkResponse::GenerateInvalidRequestResponse(
    NetworkResponseDataForInvalidRequest* newddata)
{
	ResponseType = NETWORKRESPONSETYPE_INVALIDREQUEST;
	// Destroy old data if any //
	SAFE_DELETE(ResponseData);

	ResponseData = newddata;
}

DLLEXPORT void NetworkResponse::GenerateServerStatusResponse(
    NetworkResponseDataForServerStatus* newddata)
{
	ResponseType = NETWORKRESPONSETYPE_SERVERSTATUS;
	// Destroy old data if any //
	SAFE_DELETE(ResponseData);

	ResponseData = newddata;
}

DLLEXPORT void NetworkResponse::GenerateServerDisallowResponse(
    NetworkResponseDataForServerDisallow* newddata)
{
	ResponseType = NETWORKRESPONSETYPE_SERVERDISALLOW;
	// Destroy old data if any //
	SAFE_DELETE(ResponseData);

	ResponseData = newddata;
}

DLLEXPORT void NetworkResponse::GenerateServerAllowResponse(
    NetworkResponseDataForServerAllow* newddata)
{
	ResponseType = NETWORKRESPONSETYPE_SERVERALLOW;
	// Destroy old data if any //
	SAFE_DELETE(ResponseData);

	ResponseData = newddata;
}

DLLEXPORT void NetworkResponse::GenerateValueSyncResponse(
    NetworkResponseDataForSyncValData* newddata)
{
	ResponseType = NETWORKRESPONSETYPE_SYNCVALDATA;
	// Destroy old data if any //
	SAFE_DELETE(ResponseData);

	ResponseData = newddata;
}

DLLEXPORT void NetworkResponse::GenerateValueSyncEndResponse(
    NetworkResponseDataForSyncDataEnd* newddata)
{
	ResponseType = NETWORKRESPONSETYPE_SYNCDATAEND;
	// Destroy old data if any //
	SAFE_DELETE(ResponseData);

	ResponseData = newddata;
}

DLLEXPORT void NetworkResponse::GenerateResourceSyncResponse(
    const char* dataptr, size_t datasize)
{
	ResponseType = NETWORKRESPONSETYPE_SYNCRESOURCEDATA;
	// Destroy old data if any //
	SAFE_DELETE(ResponseData);

	ResponseData = new NetworkResponseDataForSyncResourceData(string(dataptr, datasize));
}

DLLEXPORT void NetworkResponse::GenerateCreateNetworkedInputResponse(
    NetworkResponseDataForCreateNetworkedInput* newddata)
{
	ResponseType = NETWORKRESPONSETYPE_CREATENETWORKEDINPUT;
	// Destroy old data if any //
	SAFE_DELETE(ResponseData);

	ResponseData = newddata;
}

DLLEXPORT void NetworkResponse::GenerateUpdateNetworkedInputResponse(
    NetworkResponseDataForUpdateNetworkedInput* newddata)
{
	ResponseType = NETWORKRESPONSETYPE_UPDATENETWORKEDINPUT;
	// Destroy old data if any //
	SAFE_DELETE(ResponseData);

	ResponseData = newddata;
}

DLLEXPORT void NetworkResponse::GenerateInitialEntityResponse(
    NetworkResponseDataForInitialEntity* newddata)
{
	ResponseType = NETWORKRESPONSETYPE_INITIAL_ENTITY;
	// Destroy old data if any //
	SAFE_DELETE(ResponseData);

	ResponseData = newddata;
}

DLLEXPORT void NetworkResponse::GenerateEntityConstraintResponse(
    NetworkResponseDataForEntityConstraint* newddata)
{
    ResponseType = NETWORKRESPONSETYPE_ENTITY_CONSTRAINT;
	// Destroy old data if any //
	SAFE_DELETE(ResponseData);

	ResponseData = newddata;
}

DLLEXPORT void NetworkResponse::GenerateWorldFrozenResponse(
    NetworkResponseDataForWorldFrozen* newddata)
{

    ResponseType = NETWORKRESPONSETYPE_WORLD_FROZEN;
    // Destroy old data if any //
	SAFE_DELETE(ResponseData);

	ResponseData = newddata;
}

DLLEXPORT void NetworkResponse::GenerateEntityUpdateResponse(
    NetworkResponseDataForEntityUpdate* newddata)
{

    ResponseType = NETWORKRESPONSETYPE_ENTITY_UPDATE;
    // Destroy old data if any //
	SAFE_DELETE(ResponseData);

	ResponseData = newddata;    
}

DLLEXPORT void NetworkResponse::GenerateEntityDestructionResponse(
    NetworkResponseDataForEntityDestruction* newddata)
{
    ResponseType = NETWORKRESPONSETYPE_ENTITY_DESTRUCTION;
    // Destroy old data if any //
	SAFE_DELETE(ResponseData);

	ResponseData = newddata;
}

DLLEXPORT void NetworkResponse::GenerateAICacheUpdatedResponse(
    NetworkResponseDataForAICacheUpdated* newddata)
{
    ResponseType = NETWORKRESPONSETYPE_AI_CACHE_UPDATED;
    // Destroy old data if any //
	SAFE_DELETE(ResponseData);

	ResponseData = newddata;
}

DLLEXPORT void NetworkResponse::GenerateAICacheRemovedResponse(
    NetworkResponseDataForAICacheRemoved* newddata)
{
    ResponseType = NETWORKRESPONSETYPE_AI_CACHE_REMOVED;
    // Destroy old data if any //
	SAFE_DELETE(ResponseData);

	ResponseData = newddata;
}

DLLEXPORT void NetworkResponse::GenerateDisconnectInputResponse(
    NetworkResponseDataForDisconnectInput* newddata)
{
    ResponseType = NETWORKRESPONSETYPE_DISCONNECTINPUT;
    // Destroy old data if any //
	SAFE_DELETE(ResponseData);

	ResponseData = newddata;
}
// ------------------------------------ //
DLLEXPORT void NetworkResponse::GenerateKeepAliveResponse(){
	ResponseType = NETWORKRESPONSETYPE_KEEPALIVE;
	// Destroy old data if any //
	SAFE_DELETE(ResponseData);
}

DLLEXPORT void NetworkResponse::GenerateCloseConnectionResponse(){
	ResponseType = NETWORKRESPONSETYPE_CLOSECONNECTION;
	// Destroy old data if any //
	SAFE_DELETE(ResponseData);
}

DLLEXPORT void NetworkResponse::GenerateRemoteConsoleOpenedResponse(){
	ResponseType = NETWORKRESPONSETYPE_REMOTECONSOLEOPENED;
	// Destroy old data if any //
	SAFE_DELETE(ResponseData);
}

DLLEXPORT void NetworkResponse::GenerateRemoteConsoleClosedResponse(){
	ResponseType = NETWORKRESPONSETYPE_REMOTECONSOLECLOSED;
	// Destroy old data if any //
	SAFE_DELETE(ResponseData);
}

DLLEXPORT void NetworkResponse::GenerateHeartbeatResponse(){
	ResponseType = NETWORKRESPONSETYPE_SERVERHEARTBEAT;
	// Destroy old data if any //
	SAFE_DELETE(ResponseData);
}

DLLEXPORT void NetworkResponse::GenerateStartHeartbeatsResponse(){
	ResponseType = NETWORKRESPONSETYPE_STARTHEARTBEATS;
	// Destroy old data if any //
	SAFE_DELETE(ResponseData);
}

DLLEXPORT void NetworkResponse::GenerateEmptyResponse(){
	ResponseType = NETWORKRESPONSETYPE_NONE;
	// Destroy old data if any //
	SAFE_DELETE(ResponseData);
}
// ------------------------------------ //
DLLEXPORT void NetworkResponse::GenerateCustomResponse(
    GameSpecificPacketData* newdpacketdata)
{
	ResponseType = NETWORKRESPONSETYPE_CUSTOM;
	// Destroy old data if any //
	SAFE_DELETE(ResponseData);

	ResponseData = new NetworkResponseDataForCustom(newdpacketdata);
}

DLLEXPORT void NetworkResponse::GenerateCustomResponse(
    BaseGameSpecificResponsePacket* newdpacketdata)
{
	ResponseType = NETWORKRESPONSETYPE_CUSTOM;
	// Destroy old data if any //
	SAFE_DELETE(ResponseData);

	ResponseData = new NetworkResponseDataForCustom(
        new GameSpecificPacketData(newdpacketdata));
}
// ------------------------------------ //
DLLEXPORT sf::Packet NetworkResponse::GeneratePacketForResponse() const{

	sf::Packet generatedpacket;
	// First thing is the header //
	generatedpacket << ResponseID << (int)ResponseType;

	// We don't need to enforce the type here //
	//if(ResponseType != NETWORKRESPONSETYPE_NONE){
	if(ResponseData){
		// Add the data //
		ResponseData->AddDataToPacket(generatedpacket);
	}

	// Return the finished packet //
	return generatedpacket;
}
// ------------------------------------ //
DLLEXPORT int NetworkResponse::GetTimeOutValue() const{
	return TimeOutValue;
}

DLLEXPORT PACKET_TIMEOUT_STYLE NetworkResponse::GetTimeOutType() const{
	return TimeOutStyle;
}

DLLEXPORT int NetworkResponse::GetResponseID() const{
	return ResponseID;
}

DLLEXPORT NETWORKRESPONSETYPE NetworkResponse::GetTypeOfResponse() const{
	return ResponseType;
}

DLLEXPORT NETWORKRESPONSETYPE NetworkResponse::GetType() const{
	return ResponseType;
}
// ------------------------------------ //
DLLEXPORT NetworkResponseDataForIdentificationString* NetworkResponse::
GetResponseDataForIdentificationString() const
{
	if(ResponseType == NETWORKRESPONSETYPE_IDENTIFICATIONSTRINGS && ResponseData)
		return static_cast<NetworkResponseDataForIdentificationString*>(ResponseData);
	return NULL;
}

DLLEXPORT NetworkResponseDataForServerStatus* NetworkResponse::GetResponseDataForServerStatus() const{
	if(ResponseType == NETWORKRESPONSETYPE_SERVERSTATUS && ResponseData)
		return static_cast<NetworkResponseDataForServerStatus*>(ResponseData);
	return NULL;
}

DLLEXPORT NetworkResponseDataForSyncValData* NetworkResponse::GetResponseDataForValueSyncResponse() const{
	if(ResponseType == NETWORKRESPONSETYPE_SYNCVALDATA && ResponseData)
		return static_cast<NetworkResponseDataForSyncValData*>(ResponseData);
	return NULL;
}

DLLEXPORT NetworkResponseDataForSyncDataEnd* NetworkResponse::GetResponseDataForValueSyncEndResponse() const{
	if(ResponseType == NETWORKRESPONSETYPE_SYNCDATAEND && ResponseData)
		return static_cast<NetworkResponseDataForSyncDataEnd*>(ResponseData);
	return NULL;
}

DLLEXPORT NetworkResponseDataForCustom* NetworkResponse::GetResponseDataForGameSpecific() const{
	if(ResponseType == NETWORKRESPONSETYPE_CUSTOM && ResponseData)
		return static_cast<NetworkResponseDataForCustom*>(ResponseData);
	return NULL;
}

DLLEXPORT NetworkResponseDataForSyncResourceData* NetworkResponse::GetResponseDataForSyncResourceResponse()
    const
{
	if(ResponseType == NETWORKRESPONSETYPE_SYNCRESOURCEDATA && ResponseData)
		return static_cast<NetworkResponseDataForSyncResourceData*>(ResponseData);
	return NULL;
}

DLLEXPORT NetworkResponseDataForServerAllow* NetworkResponse::GetResponseDataForServerAllowResponse() const{
	if(ResponseType == NETWORKRESPONSETYPE_SERVERALLOW && ResponseData)
		return static_cast<NetworkResponseDataForServerAllow*>(ResponseData);
	return NULL;
}

DLLEXPORT NetworkResponseDataForCreateNetworkedInput* NetworkResponse::
GetResponseDataForCreateNetworkedInputResponse() const
{
	if(ResponseType == NETWORKRESPONSETYPE_CREATENETWORKEDINPUT && ResponseData)
		return static_cast<NetworkResponseDataForCreateNetworkedInput*>(ResponseData);
	return NULL;
}

DLLEXPORT NetworkResponseDataForUpdateNetworkedInput* NetworkResponse::
GetResponseDataForUpdateNetworkedInputResponse() const
{
	if(ResponseType == NETWORKRESPONSETYPE_UPDATENETWORKEDINPUT && ResponseData)
		return static_cast<NetworkResponseDataForUpdateNetworkedInput*>(ResponseData);
	return NULL;
}

DLLEXPORT NetworkResponseDataForEntityDestruction*
    NetworkResponse::GetResponseDataForEntityDestruction()
    const
{
	if(ResponseType == NETWORKRESPONSETYPE_ENTITY_DESTRUCTION && ResponseData)
		return static_cast<NetworkResponseDataForEntityDestruction*>(ResponseData);
	return NULL;
}

DLLEXPORT NetworkResponseDataForInitialEntity*
    NetworkResponse::GetResponseDataForInitialEntity() const
{

    if(ResponseType == NETWORKRESPONSETYPE_INITIAL_ENTITY && ResponseData)
        return static_cast<NetworkResponseDataForInitialEntity*>(ResponseData);
    return NULL;
}

DLLEXPORT NetworkResponseDataForEntityUpdate*
    NetworkResponse::GetResponseDataForEntityUpdate() const
{

    if(ResponseType == NETWORKRESPONSETYPE_ENTITY_UPDATE && ResponseData)
        return static_cast<NetworkResponseDataForEntityUpdate*>(ResponseData);
    return NULL;
}

DLLEXPORT NetworkResponseDataForEntityConstraint*
    NetworkResponse::GetResponseDataForEntityConstraint()
    const
{
    if(ResponseType == NETWORKRESPONSETYPE_ENTITY_CONSTRAINT && ResponseData)
        return static_cast<NetworkResponseDataForEntityConstraint*>(ResponseData);
    return NULL;
}

DLLEXPORT NetworkResponseDataForWorldFrozen*
    NetworkResponse::GetResponseDataForWorldFrozen() const
{
    
    if(ResponseType == NETWORKRESPONSETYPE_WORLD_FROZEN && ResponseData)
        return static_cast<NetworkResponseDataForWorldFrozen*>(ResponseData);
    return NULL;
}

DLLEXPORT NetworkResponseDataForAICacheUpdated*
    NetworkResponse::GetResponseDataForAICacheUpdated() const
{

    if(ResponseType == NETWORKRESPONSETYPE_AI_CACHE_UPDATED && ResponseData)
        return static_cast<NetworkResponseDataForAICacheUpdated*>(ResponseData);
    return NULL;
}

DLLEXPORT NetworkResponseDataForAICacheRemoved*
    NetworkResponse::GetResponseDataForAICacheRemoved() const
{

    if(ResponseType == NETWORKRESPONSETYPE_AI_CACHE_REMOVED && ResponseData)
        return static_cast<NetworkResponseDataForAICacheRemoved*>(ResponseData);
    return NULL;
}

DLLEXPORT NetworkResponseDataForDisconnectInput*
NetworkResponse::GetResponseDataForDisconnectInput() const
{
    if(ResponseType == NETWORKRESPONSETYPE_DISCONNECTINPUT && ResponseData)
        return static_cast<NetworkResponseDataForDisconnectInput*>(ResponseData);
    return NULL;
}
// ------------------------------------ //
DLLEXPORT
    NetworkResponseDataForIdentificationString::NetworkResponseDataForIdentificationString(
    sf::Packet &frompacket)
{
	// Extract the data from the packet //
    frompacket >> UserReadableData >> GameName >> GameVersionString >> LeviathanVersionString;

    if(!frompacket)
        throw InvalidArgument("invalid packet format");
}

DLLEXPORT
    NetworkResponseDataForIdentificationString::NetworkResponseDataForIdentificationString(
    const std::string &userreadableidentification, const std::string &gamename,
    const std::string &gameversion, const std::string &leviathanversion) :
    UserReadableData(userreadableidentification), GameName(gamename),
    GameVersionString(gameversion),
    LeviathanVersionString(leviathanversion)
{

}

DLLEXPORT void NetworkResponseDataForIdentificationString::AddDataToPacket(sf::Packet &packet){
	packet << UserReadableData << GameName << GameVersionString << LeviathanVersionString;
}
// ------------------ NetworkResponseDataForInvalidRequest ------------------ //
DLLEXPORT NetworkResponseDataForInvalidRequest::NetworkResponseDataForInvalidRequest(
    NETWORKRESPONSE_INVALIDREASON reason, const std::string &additional /*= std::string()*/) :
    Invalidness(reason), AdditionalInfo(additional)
{

}

DLLEXPORT NetworkResponseDataForInvalidRequest::NetworkResponseDataForInvalidRequest(
    sf::Packet &frompacket)
{
	// Extract the data from the packet //
	int tmpextract;
	frompacket >> tmpextract;

	Invalidness = static_cast<NETWORKRESPONSE_INVALIDREASON>(tmpextract);

    frompacket >> AdditionalInfo;

    if(!frompacket)
        throw InvalidArgument("invalid packet format");
}

DLLEXPORT void NetworkResponseDataForInvalidRequest::AddDataToPacket(sf::Packet &packet){
	packet << static_cast<int>(Invalidness) << AdditionalInfo;
}
// ------------------ NetworkResponseDataForServerStatus ------------------ //
DLLEXPORT NetworkResponseDataForServerStatus::NetworkResponseDataForServerStatus(
    const std::string &servername,
    bool isjoinable, NETWORKRESPONSE_SERVERJOINRESTRICT whocanjoin, int players,
    int maxplayers, int bots,
    NETWORKRESPONSE_SERVERSTATUS currentstatus, int serverflags) :
    ServerNameString(servername), Joinable(isjoinable), JoinRestriction(whocanjoin),
    Players(players),
    MaxPlayers(maxplayers), Bots(bots), ServerStatus(currentstatus),
    AdditionalFlags(serverflags)
{
	// Check the length //
	if(ServerNameString.length() > 100){

		Logger::Get()->Warning("NetworkResponse: NetworkResponseDataForServerStatus: server "
            "name is too long, max 100 characters (is "+Convert::ToString(
                ServerNameString.length())+") : "+ServerNameString+" will be truncated:");
		ServerNameString.resize(100);
		Logger::Get()->Write("\t> "+ServerNameString+"\n");
	}
}

DLLEXPORT NetworkResponseDataForServerStatus::NetworkResponseDataForServerStatus(
    sf::Packet &frompacket)
{
	// Try to reverse the process //
	int tmpextract;

	frompacket >> ServerNameString  >> Joinable >> tmpextract >> Players >> MaxPlayers;
    frompacket >> Bots >> tmpextract >> AdditionalFlags;
        
    // Just in case we got an invalid packet check this //
	if(ServerNameString.size() > 100){

		ServerNameString.resize(100);
		Logger::Get()->Warning("NetworkResponseDataForServerStatus: packet had too "
            "long server name string");
	}

    JoinRestriction = static_cast<NETWORKRESPONSE_SERVERJOINRESTRICT>(tmpextract);    
	ServerStatus = static_cast<NETWORKRESPONSE_SERVERSTATUS>(tmpextract);

    if(!frompacket)
        throw InvalidArgument("invalid packet format");
}

DLLEXPORT void NetworkResponseDataForServerStatus::AddDataToPacket(sf::Packet &packet){
	packet << ServerNameString << Joinable << static_cast<int>(JoinRestriction) <<
        Players << MaxPlayers << Bots <<
        static_cast<int>(ServerStatus) << AdditionalFlags;
}
// ------------------ NetworkResponseDataForServerDisallow ------------------ //
DLLEXPORT NetworkResponseDataForServerDisallow::NetworkResponseDataForServerDisallow(
    sf::Packet &frompacket)
{
	int tmpextract;

    frompacket >> tmpextract >> Message;

    if(!frompacket)
        throw InvalidArgument("invalid packet format");

    Reason = static_cast<NETWORKRESPONSE_INVALIDREASON>(tmpextract);

	// Just in case we got an invalid packet check this //
	if(Message.size() > 100){

		Message.resize(100);
		Logger::Get()->Warning("NetworkResponseDataForServerDisallow: packet had too "
            "long message string");
	}
}

DLLEXPORT NetworkResponseDataForServerDisallow::NetworkResponseDataForServerDisallow(
    NETWORKRESPONSE_INVALIDREASON reason, const std::string &message
    /*= "Default disallow"*/) :
    Message(message), Reason(reason)
{
	// Check the length //
	if(Message.length() > 100){

		LOG_WARNING("NetworkResponse: NetworkResponseDataForServerDisallow: message "
            "is too long (is "+Convert::ToString(Message.length())+") : "+
            Message+" will be truncated:");
		Message.resize(100);
		Logger::Get()->Write("\t> "+Message+"\n");
	}
}

DLLEXPORT void NetworkResponseDataForServerDisallow::AddDataToPacket(sf::Packet &packet){
	packet << Reason << Message;
}
// ------------------ NetworkResponseDataForServerAllow ------------------ //
DLLEXPORT NetworkResponseDataForServerAllow::NetworkResponseDataForServerAllow(
    sf::Packet &frompacket)
{
	int tmpextract;

    frompacket >> tmpextract >> Message;

    if(!frompacket)
        throw InvalidArgument("invalid packet format");

	ServerAcceptedWhat = static_cast<NETWORKRESPONSE_SERVERACCEPTED_TYPE>(tmpextract);
    
	// Just in case we got an invalid packet check this //
	if(Message.size() > 100){

		Message.resize(100);
		Logger::Get()->Warning("NetworkResponseDataForServerAllow: packet had too "
            "long message string");
	}
}

DLLEXPORT NetworkResponseDataForServerAllow::NetworkResponseDataForServerAllow(
    NETWORKRESPONSE_SERVERACCEPTED_TYPE whataccepted, const std::string &message /*= ""*/) :
    ServerAcceptedWhat(whataccepted), Message(message)
{
	// Check the length //
	if(Message.length() > 100){

		Logger::Get()->Warning("NetworkResponse: NetworkResponseDataForServerAllow: message "
            "is too long (is "+Convert::ToString(Message.length())+") : "+
            Message+" will be truncated:");
		Message.resize(100);
		Logger::Get()->Write("\t> "+Message+"\n");
	}
}

DLLEXPORT void NetworkResponseDataForServerAllow::AddDataToPacket(sf::Packet &packet){
	packet << ServerAcceptedWhat << Message;
}
// ------------------ NetworkResponseDataForSyncValData ------------------ //
DLLEXPORT NetworkResponseDataForSyncValData::NetworkResponseDataForSyncValData(
    NamedVariableList* newddata) :
    SyncValueData(newddata)
{

}

DLLEXPORT NetworkResponseDataForSyncValData::NetworkResponseDataForSyncValData(
    sf::Packet &frompacket)
{
	// Create new value from the packet //
	SyncValueData = std::shared_ptr<NamedVariableList>(new NamedVariableList(frompacket));
}

DLLEXPORT void NetworkResponseDataForSyncValData::AddDataToPacket(sf::Packet &packet){
	// Pass the data to the packet //
	SyncValueData->AddDataToPacket(packet);
}
// ------------------ NetworkResponseDataForSyncDataEnd ------------------ //
DLLEXPORT NetworkResponseDataForSyncDataEnd::NetworkResponseDataForSyncDataEnd(
    sf::Packet &frompacket)
{
	if(!(frompacket >> Succeeded)){
        
        throw InvalidArgument("invalid packet format");
	}
}

DLLEXPORT NetworkResponseDataForSyncDataEnd::NetworkResponseDataForSyncDataEnd(
    bool succeeded) : Succeeded(succeeded)
{

}

DLLEXPORT void NetworkResponseDataForSyncDataEnd::AddDataToPacket(sf::Packet &packet){
	packet << Succeeded;
}
// ------------------ NetworkResponseDataForGameSpecific ------------------ //
DLLEXPORT NetworkResponseDataForCustom::NetworkResponseDataForCustom(
    GameSpecificPacketData* newdpacketdata)
    : ActualPacketData(newdpacketdata)
{

}

DLLEXPORT NetworkResponseDataForCustom::NetworkResponseDataForCustom(
    BaseGameSpecificResponsePacket* newddata) : 
	ActualPacketData(new GameSpecificPacketData(newddata))
{

}

DLLEXPORT NetworkResponseDataForCustom::NetworkResponseDataForCustom(sf::Packet &frompacket){
	ActualPacketData =
        GameSpecificPacketHandler::Get()->ReadGameSpecificPacketFromPacket(true, frompacket);
	if(!ActualPacketData){

        throw InvalidArgument("invalid packet format for user defined response");
	}
}

DLLEXPORT void NetworkResponseDataForCustom::AddDataToPacket(sf::Packet &packet){
	GameSpecificPacketHandler::Get()->PassGameSpecificDataToPacket(ActualPacketData.get(),
        packet);
}
// ------------------ NetworkResponseDataForCustom ------------------ //
DLLEXPORT NetworkResponseDataForSyncResourceData::NetworkResponseDataForSyncResourceData(
    sf::Packet &frompacket)
{
	
	if(!(frompacket >> OurCustomData)){

        throw InvalidArgument("invalid packet format");
	}
}

DLLEXPORT NetworkResponseDataForSyncResourceData::NetworkResponseDataForSyncResourceData(
    const string &containeddata) : 
	OurCustomData(containeddata)
{

}

DLLEXPORT void NetworkResponseDataForSyncResourceData::AddDataToPacket(sf::Packet &packet)
{
	packet << OurCustomData;
}
// ------------------ NetworkResponseDataForCreateNetworkedInput ------------------ //
DLLEXPORT
    NetworkResponseDataForCreateNetworkedInput::NetworkResponseDataForCreateNetworkedInput(
    sf::Packet &frompacket)
{
	// Load the packet from the packet //
	string tmpstr;
	frompacket >> tmpstr;

	// Fill the actual packet //
	DataForObject.append(tmpstr.c_str(), tmpstr.size());
}

DLLEXPORT
    NetworkResponseDataForCreateNetworkedInput::NetworkResponseDataForCreateNetworkedInput(
    NetworkedInput &tosend)
{
	// Copy the data to our packet //

	tosend.AddFullDataToPacket(DataForObject);
}

DLLEXPORT void NetworkResponseDataForCreateNetworkedInput::AddDataToPacket(sf::Packet &packet){

	packet << string(reinterpret_cast<const char*>(
            DataForObject.getData()), DataForObject.getDataSize());
}
// ------------------ NetworkResponseDataForUpdateNetworkedInput ------------------ //
DLLEXPORT
    NetworkResponseDataForUpdateNetworkedInput::NetworkResponseDataForUpdateNetworkedInput(
    sf::Packet &frompacket)
{
	// First the ID //
	if(!(frompacket >> InputID)){

        throw InvalidArgument("invalid packet format");
	}

	// Load the packet from the packet //
	string tmpstr;
	frompacket >> tmpstr;

	// Fill the actual packet //
	UpdateData.append(tmpstr.c_str(), tmpstr.size());
}

DLLEXPORT
    NetworkResponseDataForUpdateNetworkedInput::NetworkResponseDataForUpdateNetworkedInput(
    NetworkedInput &object)
{
	// Get the ID first //
	InputID = object.GetID();

	// Then copy the data to our packet //
	object.AddChangesToPacket(UpdateData);
}

DLLEXPORT
    NetworkResponseDataForUpdateNetworkedInput::NetworkResponseDataForUpdateNetworkedInput(
    const NetworkResponseDataForUpdateNetworkedInput &other) :
    InputID(other.InputID)
{
    
    UpdateData.append(other.UpdateData.getData(), other.UpdateData.getDataSize());
}

DLLEXPORT void NetworkResponseDataForUpdateNetworkedInput::AddDataToPacket(sf::Packet &packet){
	packet << InputID << string(reinterpret_cast<const char*>(
            UpdateData.getData()), UpdateData.getDataSize());
}
// ------------------ NetworkResponseDataForInitialEntity ------------------ //
DLLEXPORT NetworkResponseDataForInitialEntity::NetworkResponseDataForInitialEntity(
    sf::Packet &frompacket)
{
    // Do the reverse of AddDataToPacket... //
    uint32_t size;
    
    frompacket >> WorldID >> size;

    if(!frompacket)
        throw InvalidArgument("invalid packet format");

    EntityData.reserve(size);

    for(uint32_t i = 0; i < size; i++){
        std::string tmpstr;

        if(!(frompacket >> tmpstr)){
            
            throw InvalidArgument("invalid packet format");
        }
        
        std::shared_ptr<sf::Packet> packit = make_shared<sf::Packet>();

        packit->append(tmpstr.c_str(), tmpstr.size());

        EntityData.push_back((packit));
    }
}

DLLEXPORT NetworkResponseDataForInitialEntity::NetworkResponseDataForInitialEntity(int worldid,
    std::unique_ptr<sf::Packet> &entity1data) :
    WorldID(worldid), EntityData(1)
{
    EntityData[0] = std::shared_ptr<sf::Packet>(entity1data.release());
}
        
DLLEXPORT void NetworkResponseDataForInitialEntity::AddDataToPacket(sf::Packet &packet){
    // First put our world id //
    packet << WorldID;

    // And then the count of our entity data and shove stuff there //
    uint32_t size = static_cast<uint32_t>(EntityData.size());
    packet << size;

    for(uint32_t i = 0; i < size; i++){

        const std::string tmpstr(reinterpret_cast<const char*>(
                EntityData[i]->getData()), EntityData[i]->getDataSize());
        
        // Warn if it is quite large //
        if(tmpstr.size() >= 500){

            Logger::Get()->Warning("Sending a large object, over 500 bytes in size");
        }

        packet << tmpstr;
    }
}

DLLEXPORT std::shared_ptr<sf::Packet>  NetworkResponseDataForInitialEntity::GetDataForEntity(
    size_t index) const
{
    // Check is it out of bounds //
    if(index >= EntityData.size())
        return nullptr;

    return EntityData[index];
}
// ------------------ NetworkResponseDataForEntityConstraint ------------------ //
DLLEXPORT NetworkResponseDataForEntityConstraint::NetworkResponseDataForEntityConstraint(
    int worldid, int constraintid, int entity1, int entity2, bool create,
    ENTITY_CONSTRAINT_TYPE type, std::shared_ptr<sf::Packet> &data) :
    Create(create), WorldID(worldid), ConstraintID(constraintid),
    EntityID1(entity1), EntityID2(entity2),
    Type(type), ConstraintData(data)
{

}

DLLEXPORT NetworkResponseDataForEntityConstraint::NetworkResponseDataForEntityConstraint(
    int worldid, int constraintid, ObjectID entity1, ObjectID entity2, bool create,
    ENTITY_CONSTRAINT_TYPE type) :
    Create(create), WorldID(worldid), ConstraintID(constraintid),
    EntityID1(entity1), EntityID2(entity2), Type(type)
{

}


DLLEXPORT NetworkResponseDataForEntityConstraint::NetworkResponseDataForEntityConstraint(
    sf::Packet &frompacket)
{

    int32_t tmptype;
    frompacket >> WorldID >> ConstraintID >> EntityID1 >> EntityID2 >> Create >> tmptype;
    
    std::string tmpstr;
    frompacket >> tmpstr;

    if(!frompacket)
        throw InvalidArgument("invalid packet format");


    Type = static_cast<ENTITY_CONSTRAINT_TYPE>(tmptype);

    if(tmpstr.size() > 0){
        
        ConstraintData = make_shared<sf::Packet>();

        ConstraintData->append(tmpstr.c_str(), tmpstr.size());
    }
}

DLLEXPORT void NetworkResponseDataForEntityConstraint::AddDataToPacket(
    sf::Packet &packet)
{

    packet << WorldID << ConstraintID << EntityID1 << EntityID2 << Create <<
        static_cast<int32_t>(Type);

    // There might be no data for whatever reason //
    if(ConstraintData){
        
        const std::string tmpstr(reinterpret_cast<const char*>(ConstraintData->getData()),
            ConstraintData->getDataSize());
        
        // Warn if it is quite large //
        if(tmpstr.size() >= 500){

            Logger::Get()->Warning("Sending a large constraint, over 500 bytes in size");
        }

        packet << tmpstr;
        
    } else {

        packet << string("");
    }
}

// ------------------ NetworkResponseDataForWorldFrozen ------------------ //
DLLEXPORT NetworkResponseDataForWorldFrozen::NetworkResponseDataForWorldFrozen(
    int worldid, bool frozen, int ontick) :
    WorldID(worldid), Frozen(frozen), TickNumber(ontick)
{

}

DLLEXPORT NetworkResponseDataForWorldFrozen::NetworkResponseDataForWorldFrozen(
    sf::Packet &frompacket)
{

    frompacket >> WorldID >> Frozen >> TickNumber;

    if(!frompacket)
        throw InvalidArgument("invalid packet format");

}

DLLEXPORT void NetworkResponseDataForWorldFrozen::AddDataToPacket(sf::Packet &packet){

    packet << WorldID << Frozen << TickNumber;
}
// ------------------ NetworkResponseDataForEntityUpdate ------------------ //
DLLEXPORT NetworkResponseDataForEntityUpdate::NetworkResponseDataForEntityUpdate(
    int worldid, int entityid, int ticknumber, int referencetick,
    std::shared_ptr<sf::Packet> data) :
    WorldID(worldid), EntityID(entityid), TickNumber(ticknumber),
    ReferenceTick(referencetick), UpdateData(data)
{

}

DLLEXPORT NetworkResponseDataForEntityUpdate::NetworkResponseDataForEntityUpdate(
    sf::Packet &frompacket)
{

    frompacket >> WorldID >> EntityID >> TickNumber >> ReferenceTick;
    
    std::string tmpstr;
    frompacket >> tmpstr;

    if(!frompacket)
        throw InvalidArgument("invalid packet format");

    UpdateData = make_shared<sf::Packet>();

    UpdateData->append(tmpstr.c_str(), tmpstr.size());
}

DLLEXPORT void NetworkResponseDataForEntityUpdate::AddDataToPacket(sf::Packet &packet){

    packet << WorldID << EntityID << TickNumber << ReferenceTick;

    const std::string tmpstr(reinterpret_cast<const char*>(
            UpdateData->getData()), UpdateData->getDataSize());
        
    // Warn if it is quite large //
    if(tmpstr.size() >= 500){

        Logger::Get()->Warning("Sending a large entity update, over 500 bytes in size");
    }

    packet << tmpstr;
}
// ------------------ NetworkResponseDataForEntityDestruction ------------------ //
DLLEXPORT NetworkResponseDataForEntityDestruction::NetworkResponseDataForEntityDestruction(
    int worldid, int entityid) : WorldID(worldid), EntityID(entityid)
{

}

DLLEXPORT NetworkResponseDataForEntityDestruction::NetworkResponseDataForEntityDestruction(
    sf::Packet &frompacket)
{
    frompacket >> WorldID >> EntityID;

    if(!frompacket)
        throw InvalidArgument("invalid packet format");
}

DLLEXPORT void NetworkResponseDataForEntityDestruction::AddDataToPacket(sf::Packet &packet){

    packet << WorldID << EntityID;
}
// ------------------ NetworkResponseDataForAICacheUpdated ------------------ //
DLLEXPORT NetworkResponseDataForAICacheUpdated::NetworkResponseDataForAICacheUpdated(
    std::shared_ptr<NamedVariableList> variable) :
    Variable(variable)
{
    if(!Variable)
        throw InvalidArgument("invalid packet format");
}

DLLEXPORT NetworkResponseDataForAICacheUpdated::NetworkResponseDataForAICacheUpdated(
    sf::Packet &frompacket)
{

    Variable = make_shared<NamedVariableList>(frompacket);

    if(!frompacket)
        throw InvalidArgument("invalid packet format");
}

DLLEXPORT void NetworkResponseDataForAICacheUpdated::AddDataToPacket(sf::Packet &packet){

    Variable->AddDataToPacket(packet);
}
// ------------------ NetworkResponseDataForAICacheRemoved ------------------ //
DLLEXPORT NetworkResponseDataForAICacheRemoved::NetworkResponseDataForAICacheRemoved(
    const std::string &name) : Name(name)
{
    
}

DLLEXPORT NetworkResponseDataForAICacheRemoved::NetworkResponseDataForAICacheRemoved(
    sf::Packet &frompacket)
{

    if(!(frompacket >> Name)){

        throw InvalidArgument("invalid packet format");
    }
}

DLLEXPORT void NetworkResponseDataForAICacheRemoved::AddDataToPacket(sf::Packet &packet){

    packet << Name;
    
    // Warn if it is quite large //
    if(Name.size() >= 800){

        Logger::Get()->Warning(
            "Sending a large AI cache remove, string is over 800 elements long");
    }
}
// ------------------ NetworkResponseDataForDisconnectInput ------------------ //
NetworkResponseDataForDisconnectInput::NetworkResponseDataForDisconnectInput(
    int inputid, int ownerid) :
    InputID(inputid), OwnerID(ownerid)
{

}

NetworkResponseDataForDisconnectInput::NetworkResponseDataForDisconnectInput(
    sf::Packet &packet)
{

    packet << InputID << OwnerID;
}

DLLEXPORT void NetworkResponseDataForDisconnectInput::AddDataToPacket(sf::Packet &packet){

    packet >> InputID >> OwnerID;

    if(!packet)
        throw InvalidArgument("invalid packet format");
}
