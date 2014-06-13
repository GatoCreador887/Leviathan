#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_COMMANDHANDLER
#include "CommandHandler.h"
#endif
#include "Iterators/StringIterator.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::CommandHandler::CommandHandler(NetworkServerInterface* owneraccess) : Owner(owneraccess){
	Staticaccess = this;
}

DLLEXPORT Leviathan::CommandHandler::~CommandHandler(){
	{
		boost::unique_lock<boost::mutex> lock(StaticDeleteMutex);
		Staticaccess = NULL;
	}

	GUARD_LOCK_THIS_OBJECT();

	CustomHandlers.clear();

	// All the pointers have to be removed before deleting this //
	_LetGoOfAll(guard);
}

CommandHandler* Leviathan::CommandHandler::Staticaccess;
boost::mutex Leviathan::CommandHandler::StaticDeleteMutex;


DLLEXPORT CommandHandler* Leviathan::CommandHandler::Get(unique_ptr<ObjectLock> &lockereceiver){
	boost::unique_lock<boost::mutex> lock(StaticDeleteMutex);
	if(Staticaccess){
		
		GUARD_LOCK_OTHER_OBJECT_UNIQUE_PTR_NAME(Staticaccess, olock);
		lockereceiver.swap(olock);
	}

	return Staticaccess;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::CommandHandler::QueueCommand(const string &command, CommandSender* issuer){
	GUARD_LOCK_THIS_OBJECT();

	// Get the first word //
	StringIterator itr(new UTF8DataIterator(command));

	// Only skip a single / if there is one //
	if(itr.GetCharacter() == '/')
		itr.MoveToNext();

	// Get the first word //
	auto firstword = itr.GetNextCharacterSequence<string>(UNNORMALCHARACTER_TYPE_LOWCODES);

	if(!firstword || firstword->empty())
		return;



	// Find a command handler that provides handling for this command //

	// First check the default commands //
	if(IsThisDefaultCommand(*firstword)){

		// Queue a default command execution //
		DEBUG_BREAK;

		// Default will also need this //
		_AddSender(issuer, guard);
		return;
	}

	// Loop through and check the custom command handlers //
	DEBUG_BREAK;


	// Queue the command handler //


	// And take good care of the object while the command handler is waiting //
	_AddSender(issuer, guard);
}

DLLEXPORT void Leviathan::CommandHandler::RemoveMe(CommandSender* object){
	GUARD_LOCK_THIS_OBJECT();

	// Remove from the vector //
	auto end = SendersInUse.end();
	for(auto iter = SendersInUse.begin(); iter != end; ++iter){

		if(*iter == object){
			// Remove the match //
			SendersInUse.erase(iter);
			return;
		}
	}
}

DLLEXPORT bool Leviathan::CommandHandler::IsSenderStillValid(CommandSender* checkthis, unique_ptr<ObjectLock> &retlock){
	GUARD_LOCK_THIS_OBJECT();

	// Check is it still in the list //
	auto end = SendersInUse.end();
	for(auto iter = SendersInUse.begin(); iter != end; ++iter){
		if(*iter == checkthis){
			// It is still there //
			GUARD_LOCK_OTHER_OBJECT_UNIQUE_PTR_NAME((*iter), tmplock);
			retlock.swap(tmplock);
			return true;
		}
	}

	// Not found //
	return false;
}

// ------------------------------------ //
DLLEXPORT void Leviathan::CommandHandler::UpdateStatus(){
	GUARD_LOCK_THIS_OBJECT();


}
// ------------------------------------ //
void Leviathan::CommandHandler::_LetGoOfAll(ObjectLock &guard){
	VerifyLock(guard);

	auto end = SendersInUse.end();
	for(auto iter = SendersInUse.begin(); iter != end; ++iter){

		(*iter)->EndOwnership(this);
	}

	// Clear them all at once //
	SendersInUse.clear();
}
// ------------------------------------ //
DLLEXPORT void Leviathan::CommandHandler::SenderNoLongerRequired(CommandSender* checkthis, const unique_ptr<ObjectLock> &stillgotthis){
	GUARD_LOCK_THIS_OBJECT();

	// Remove from the vector //
	auto end = SendersInUse.end();
	for(auto iter = SendersInUse.begin(); iter != end; ++iter){

		if(*iter == checkthis){
			// Notify it //
			(*iter)->EndOwnership(this);

			// Remove the match //
			SendersInUse.erase(iter);
			return;
		}
	}
}

void Leviathan::CommandHandler::_AddSender(CommandSender* object, ObjectLock &guard){
	VerifyLock(guard);

	// Notify the object //
	object->StartOwnership(this);

	// Add to the list //
	SendersInUse.push_back(object);
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::CommandHandler::RegisterCustomCommandHandler(unique_ptr<CustomCommandHandler> handler){
	// Might be unnecessary to check this, but it's a way to return false sometimes //
	if(!handler)
		return false;

	CustomHandlers.push_back(move(handler));
	return true;
}

DLLEXPORT bool Leviathan::CommandHandler::IsThisDefaultCommand(const string &firstword) const{

	// Didn't match any of the strings, cannot be //
	return false;
}
// ------------------ CommandSender ------------------ //
DLLEXPORT void Leviathan::CommandSender::StartOwnership(CommandHandler* commander){
	GUARD_LOCK_THIS_OBJECT();

	// Just add to the list //
	CommandHandlersToNotify.push_back(commander);
}

DLLEXPORT void Leviathan::CommandSender::EndOwnership(CommandHandler* which){
	GUARD_LOCK_THIS_OBJECT();

	// Find the right one and remove it //
	auto end = CommandHandlersToNotify.end();
	for(auto iter = CommandHandlersToNotify.begin(); iter != end; ++iter){

		if(*iter == which){

			// Remove the match //
			CommandHandlersToNotify.erase(iter);
			return;
		}
	}
}

DLLEXPORT void Leviathan::CommandSender::_OnReleaseParentCommanders(ObjectLock &guard){
	VerifyLock(guard);

	auto end = CommandHandlersToNotify.end();
	for(auto iter = CommandHandlersToNotify.begin(); iter != end; ++iter){

		(*iter)->RemoveMe(this);
	}

	// Clear all at once for hopefully better performance //
	CommandHandlersToNotify.clear();
}


