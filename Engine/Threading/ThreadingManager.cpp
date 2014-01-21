#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_THREADINGMANAGER
#include "ThreadingManager.h"
#endif
#include "OgreRoot.h"
#include "QueuedTask.h"
using namespace Leviathan;
// ------------------------------------ //

// ------------------ Utility functions for threads to run ------------------ //
void RegisterOgreOnThread(){

	Ogre::Root::getSingleton().getRenderSystem()->registerThread();
	Logger::Get()->Info(L"Thread registered to work with Ogre");
}


// ------------------ ThreadingManager ------------------ //
DLLEXPORT Leviathan::ThreadingManager::ThreadingManager(int basethreadspercore /*= DEFAULT_THREADS_PER_CORE*/) : AllowStartTasksFromQueue(true),
	StopProcessing(false), TaksMustBeRanBeforeState(TASK_MUSTBERAN_BEFORE_EXIT)
{
	WantedThreadCount = boost::thread::hardware_concurrency()*basethreadspercore;

	staticaccess = this;
}

DLLEXPORT Leviathan::ThreadingManager::~ThreadingManager(){

	staticaccess = NULL;
}

DLLEXPORT ThreadingManager* Leviathan::ThreadingManager::Get(){
	return staticaccess;
}

ThreadingManager* Leviathan::ThreadingManager::staticaccess = NULL;
// ------------------------------------ //
DLLEXPORT bool Leviathan::ThreadingManager::Init(){

	ObjectLock guard(*this);

	// Start the queuer //
	WorkQueueHandler = boost::thread(RunTaskQueuerThread, this);


	// Start appropriate amount of threads //

	for(int i = 0; i < WantedThreadCount; i++){


		UsableThreads.push_back(shared_ptr<TaskThread>(new TaskThread()));
	}

	// Check that at least one thread is running //
	for(auto iter = UsableThreads.begin(); iter != UsableThreads.end(); ++iter){

		if((*iter)->HasStarted())
			return true;
	}

	Logger::Get()->Error(L"ThreadingManager: Init: no threads have started");

	// No threads running //
	return false;
}

DLLEXPORT void Leviathan::ThreadingManager::Release(){
	// Disallow new tasks //
	{
		ObjectLock guard(*this);
		AllowStartTasksFromQueue = false;
	}

	// Wait for all to finish //
	WaitForAllTasksToFinish();

	{
		ObjectLock guard(*this);
		StopProcessing = true;
	}
	// Wait for the queuer to exit //
	TaskQueueNotify.notify_all();
	WorkQueueHandler.join();

	// Tell all threads to quit //
	for(auto iter = UsableThreads.begin(); iter != UsableThreads.end(); ++iter){
			(*iter)->NotifyKill();
	}
}
// ------------------------------------ //
DLLEXPORT void Leviathan::ThreadingManager::QueueTask(shared_ptr<QueuedTask> task){
	{
		ObjectLock guard(*this);

		WaitingTasks.push_back(task);
	}
	// Notify the thread //
	TaskQueueNotify.notify_all();
}

DLLEXPORT void Leviathan::ThreadingManager::FlushActiveThreads(){
	// Disallow new tasks //
	{
		ObjectLock guard(*this);
		AllowStartTasksFromQueue = false;
	}

	// Wait until all threads are available again //
	boost::unique_lock<ThreadSafe> lockit(*this);

	bool allavailable = false;

	// We want to skip wait on loop //
	goto skipfirstwaitforthreadslabel;

	while(!allavailable){

		// Wait for tasks to update //
		TaskQueueNotify.wait(lockit);

skipfirstwaitforthreadslabel:


		// Set to true until a thread is busy //
		allavailable = true;

		for(auto iter = UsableThreads.begin(); iter != UsableThreads.end(); ++iter){
			if((*iter)->HasRunningTask()){
				allavailable = false;
				break;
			}
		}
	}

	// Now free //
}

DLLEXPORT void Leviathan::ThreadingManager::WaitForAllTasksToFinish(){
	// Use this lock the entire function //
	boost::unique_lock<ThreadSafe> lockit(*this);

	// See if empty right now and loop until it is //
	while(WaitingTasks.size() != 0){

		// Wait for update //
		TaskQueueNotify.wait(lockit);
	}

	// Wait for threads to empty up //
	bool allavailable = false;

	// We want to skip wait on loop //
	goto skipfirstwaitforthreadslabel2;

	while(!allavailable){

		// Wait for tasks to update //
		try{
			TaskQueueNotify.timed_wait(lockit, boost::posix_time::microseconds((50)));
		} catch(...){

		}

skipfirstwaitforthreadslabel2:


		// Set to true until a thread is busy //
		allavailable = true;

		for(auto iter = UsableThreads.begin(); iter != UsableThreads.end(); ++iter){
			if((*iter)->HasRunningTask()){
				allavailable = false;
				break;
			}
		}
	}

}

DLLEXPORT void Leviathan::ThreadingManager::NotifyTaskFinished(shared_ptr<QueuedTask> task){
	// We need locking for re-adding it //
	if(task->IsRepeating()){
		// Add back to queue //
		ObjectLock guard(*this);

		WaitingTasks.push_back(task);
	}


	// We probably don't need to acquire a lock for this //
	TaskQueueNotify.notify_all();
}
// ------------------------------------ //
DLLEXPORT void Leviathan::ThreadingManager::MakeThreadsWorkWithOgre(){
	QUICKTIME_THISSCOPE;
	// Disallow new tasks //
	{
		ObjectLock guard(*this);
		AllowStartTasksFromQueue = false;
	}

	// Wait for tasks to finish //
	FlushActiveThreads();

	// All threads are now available //

	// Call pre register function //
	Ogre::Root::getSingleton().getRenderSystem()->preExtraThreadsStarted();

	// Set the threads to run the register methods //
	{
		boost::unique_lock<ThreadSafe> guard(*this);

		for(auto iter = UsableThreads.begin(); iter != UsableThreads.end(); ++iter){
			(*iter)->SetTaskAndNotify(shared_ptr<QueuedTask>(new QueuedTask(boost::bind(RegisterOgreOnThread))));
			// Wait for it to end //
#ifdef __GNUC__
			while((*iter)->HasRunningTask()){
				try{
					TaskQueueNotify.wait(guard);
				}
				catch(...){
					Logger::Get()->Warning(L"ThreadingManager: MakeThreadsWorkWithOgre: wait interrupted");
				}
			}
#endif
		}
	}

	// Wait for threads to finish //
	FlushActiveThreads();

	// End registering functions //
	Ogre::Root::getSingleton().getRenderSystem()->postExtraThreadsStarted();

	// Allow new threads //
	{
		ObjectLock guard(*this);
		AllowStartTasksFromQueue = true;
	}
}
// ------------------------------------ //
void Leviathan::RunTaskQueuerThread(ThreadingManager* manager){

	// Lock the object //
	boost::unique_lock<ThreadSafe> lockit(*manager);

	while(!manager->StopProcessing){

		// Wait until task queue needs work //
		manager->TaskQueueNotify.wait(lockit);

		// Quickly continue if it is empty //
		if(!manager->AllowStartTasksFromQueue || manager->WaitingTasks.size() == 0){

			continue;
		}

		// Keep iterator consistent with the whole loop, (to avoid excessive calling of CanBeRan) //
		auto taskiter = manager->WaitingTasks.begin();
		// Used to iterate again, but just checking if they can be ran (allows more important tasks to run first) //
		auto nonimportantiter = manager->WaitingTasks.begin();

		// Find an empty thread and queue tasks //
		for(auto iter = manager->UsableThreads.begin(); iter != manager->UsableThreads.end(); ++iter){

			if(!(*iter)->HasRunningTask()){

				// Break if no tasks //
				if(manager->WaitingTasks.size() == 0)
					break;

				// Queue a task //
				shared_ptr<QueuedTask> tmptask;

				// Try to find a suitable one //
				for( ; taskiter != manager->WaitingTasks.end(); ){

					// Check does the task want to run now //
					if((*taskiter)->MustBeRanBefore(manager->TaksMustBeRanBeforeState)){
						// Check is allowed to run //
						if((*taskiter)->CanBeRan()){
							// Run it! //
							tmptask = (*taskiter);
							// Erase, might be temporary //
							taskiter = manager->WaitingTasks.erase(taskiter);

							// Just to be safe, TODO: performance could be improved //
							nonimportantiter = taskiter;

							break;
						}
					}
					++iter;
				}

				if(!tmptask){
					// Check with the other iterator, too //
					for( ; nonimportantiter != manager->WaitingTasks.end(); ){
						// Check does the task want to run now //
						if((*nonimportantiter)->MustBeRanBefore(manager->TaksMustBeRanBeforeState)){
							// Check is allowed to run //
							if((*nonimportantiter)->CanBeRan()){
								// Run it! //
								tmptask = (*nonimportantiter);
								// Erase, might be temporary //
								nonimportantiter = manager->WaitingTasks.erase(nonimportantiter);

								// Just to be safe, TODO: performance could be improved //
								taskiter = nonimportantiter;

								break;
							}
						}
						++iter;
					}
				}

				// If still nothing, nothing cannot run //
				if(!tmptask)
					break;

				// This won't actually finish it so to re-queue it, if it repeats, we use the callback called when it is finished //
				(*iter)->SetTaskAndNotify(tmptask);
			}
		}
	}
}

