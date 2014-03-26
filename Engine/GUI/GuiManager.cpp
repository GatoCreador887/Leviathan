#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_GUI_MAIN
#include "GuiManager.h"
#endif
#include "Engine.h"
#include "Script/ScriptInterface.h"
#include "FileSystem.h"
#include "Rendering/Graphics.h"
#include <boost/assign/list_of.hpp>
#include "Rendering/GUI/FontManager.h"
#include "include/cef_sandbox_win.h"
#include "GuiCollection.h"
#include "Common/DataStoring/DataStore.h"
#include "Common/DataStoring/DataBlock.h"
#include "Common/GraphicalInputEntity.h"
#include "Common/Window.h"
#include "OgreSceneManager.h"
#include "OgreRoot.h"
#include "OgreManualObject.h"
#include "OgreHardwarePixelBuffer.h"
#include "GuiView.h"
using namespace Leviathan;
using namespace Leviathan::Gui;
// ------------------------------------ //
Leviathan::Gui::GuiManager::GuiManager() : ID(IDFactory::GetID()), Visible(true), GuiMouseUseUpdated(true), GuiDisallowMouseCapture(true),
	MouseQuad(NULL)
{
	
}
Leviathan::Gui::GuiManager::~GuiManager(){

	
}
// ------------------------------------ //
bool Leviathan::Gui::GuiManager::Init(AppDef* vars, Graphics* graph, GraphicalInputEntity* window){
	ObjectLock guard(*this);

	ThisWindow = window;

	Window* wind = window->GetWindow();
	
	// Create Ogre resources //
	if(!_CreateInternalOgreResources(window->GetWindow()->GetOverlayScene())){

		Logger::Get()->Error(L"GuiManager: Init: failed to create internal Ogre resources");
		return false;
	}
	
	// All rendering is now handled by individual Views and the Window's Ogre scene //


	return true;
}

void Leviathan::Gui::GuiManager::Release(){
	ObjectLock guard(*this);
	// default mouse back //
	SetMouseFile(L"none");

	// Destroy the views //
	for(size_t i = 0; i < ThissViews.size(); i++){

		ThissViews[i]->ReleaseResources();
		ThissViews[i]->Release();
	}

	for(unsigned int i = 0; i < Objects.size(); i++){
		// object's release function will do everything needed (even deleted if last reference) //
		SAFE_RELEASE(Objects[i]);
	}
	Objects.clear();

	// GuiCollections are now also reference counted //
	for(size_t i = 0; i < Collections.size(); i++){
		SAFE_RELEASE(Collections[i]);
	}

	Collections.clear();

	_ReleaseOgreResources();
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Gui::GuiManager::ProcessKeyDown(OIS::KeyCode key, int specialmodifiers){
	ObjectLock guard(*this);
	for(size_t i = 0; i < Collections.size(); i++){
		if(Collections[i]->GetTogglingKey().Match(key, specialmodifiers, false) && Collections[i]->GetAllowEnable()){
			// is a match, toggle //
			Collections[i]->ToggleState();

			return true;
		}
	}


	return false;
}

DLLEXPORT void Leviathan::Gui::GuiManager::SetCollectionStateProxy(string name, bool state){
	// call the actual function //
	SetCollectionState(Convert::StringToWstring(name), state);
}

DLLEXPORT void Leviathan::Gui::GuiManager::SetCollectionState(const wstring &name, bool state){
	ObjectLock guard(*this);
	// find collection with name and set it's state //
	for(size_t i = 0; i < Collections.size(); i++){
		if(Collections[i]->GetName() == name){
			// set state //
			if(Collections[i]->GetState() != state){
				Logger::Get()->Info(L"Setting Collection "+Collections[i]->GetName()+L" state "+Convert::ToWstring(state));
				Collections[i]->ToggleState();
			}
			return;
		}
	}
	// Complain //
	Logger::Get()->Warning(L"GuiManager: SetCollectionState: couldn't find collection with name: "+name);
}

DLLEXPORT void Leviathan::Gui::GuiManager::SetCollectionAllowEnableState(const wstring &name, bool allow /*= true*/){
	ObjectLock guard(*this);
	// find collection with name and set it's allow enable state //
	for(size_t i = 0; i < Collections.size(); i++){
		if(Collections[i]->GetName() == name){
			// set state //
			if(Collections[i]->GetAllowEnable() != allow){
				Logger::Get()->Info(L"Setting Collection "+Collections[i]->GetName()+L" allow enable state "+Convert::ToWstring(allow));
				Collections[i]->ToggleAllowEnable();
			}
			return;
		}
	}
}
// ------------------------------------ //
void Leviathan::Gui::GuiManager::GuiTick(int mspassed){
	ObjectLock guard(*this);

	// check if we want mouse //
	if(GuiMouseUseUpdated){
		GuiMouseUseUpdated = false;

		// scan if any collections keep GUI active //
		bool active = false;


		for(size_t i = 0; i < Collections.size(); i++){

			if(Collections[i]->KeepsGUIActive()){
				active = true;
				break;
			}
		}

		if(active != GuiDisallowMouseCapture){
			// state updated //
			GuiDisallowMouseCapture = active;

			if(GuiDisallowMouseCapture){
				// disable mouse capture //
				ThisWindow->SetMouseCapture(false);

			} else {

				// activate direct mouse capture //
				if(!ThisWindow->SetMouseCapture(true)){
					// failed, GUI must be forced to stay on //
					OnForceGUIOn();
					GuiDisallowMouseCapture = true;
					GuiMouseUseUpdated = true;
				}
			}
		}
	}
}

DLLEXPORT void Leviathan::Gui::GuiManager::OnForceGUIOn(){
	DEBUG_BREAK;
}

void Leviathan::Gui::GuiManager::Render(){
	ObjectLock guard(*this);
	// Update inputs //
	if(ThissViews.size())
		ThisWindow->GetWindow()->GatherInput(ThissViews[0]->GetBrowserHost());

	// Update browser textures //
	for(size_t i = 0; i < ThissViews.size(); i++){

		ThissViews[i]->CheckRender();
	}
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Gui::GuiManager::OnResize(){
	ObjectLock guard(*this);

	// Resize all CEF browsers on this window //
	for(size_t i = 0; i < ThissViews.size(); i++){
		ThissViews[i]->NotifyWindowResized();
	}
}

DLLEXPORT void Leviathan::Gui::GuiManager::OnFocusChanged(bool focused){
	ObjectLock guard(*this);

	// Notify all CEF browsers on this window //
	for(size_t i = 0; i < ThissViews.size(); i++){
		ThissViews[i]->NotifyFocusUpdate(focused);
	}
}
// ------------------------------------ //
bool Leviathan::Gui::GuiManager::AddGuiObject(BaseGuiObject* obj){
	ObjectLock guard(*this);
	Objects.push_back(obj);
	return true;
}

void Leviathan::Gui::GuiManager::DeleteObject(int id){
	ObjectLock guard(*this);
	for(size_t i = 0; i < Objects.size(); i++){
		if(Objects[i]->GetID() == id){

			SAFE_RELEASE(Objects[i]);
			Objects.erase(Objects.begin()+i);
			return;
		}
	}
}

int Leviathan::Gui::GuiManager::GetObjectIndexFromId(int id){
	ObjectLock guard(*this);
	for(size_t i = 0; i < Objects.size(); i++){
		if(Objects[i]->GetID() == id)
			return i;
	}
	return -1;
}

BaseGuiObject* Leviathan::Gui::GuiManager::GetObject(unsigned int index){
	ObjectLock guard(*this);
	ARR_INDEX_CHECK(index, Objects.size()){
		return Objects[index];
	}
	return NULL;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Gui::GuiManager::LoadGUIFile(const wstring &file){
	// header flag definitions //
	vector<shared_ptr<NamedVariableList>> headerdata;

	// parse file to structure //
	vector<shared_ptr<ObjectFileObject>> data = ObjectFileProcessor::ProcessObjectFile(file, headerdata);

	NamedVars varlist(headerdata);

	// we need to load the corresponding rocket file first //
	wstring relativepath;
	// get path //
	ObjectFileProcessor::LoadValueFromNamedVars<wstring>(varlist, L"GUIBasePage", relativepath, L"", true,
		L"GuiManager: LoadGUIFile: no base page defined (in "+file+L") : ");

	if(!relativepath.size()){

		return false;
	}

	// Create the view //
	Gui::View* LoadingView = new Gui::View(this, ThisWindow->GetWindow());

	// We store a reference //
	LoadingView->AddRef();

	wstring path = StringOperations::GetPathWstring(file);

	// Create the final page //
	wstring finalpath;

	// If this is an internet page pass it unmodified //
	if(relativepath.find(L"http://") < 2){

		finalpath = relativepath;
	} else {
		// Local file, add to the end //
		finalpath = L"file:///"+path+relativepath;
	}

	// Initialize it //
	if(!LoadingView->Init(finalpath, varlist)){

		LoadingView->ReleaseResources();
		LoadingView->Release();

		Logger::Get()->Error(L"GuiManager: LoadGUIFile: failed to initialize view for file: "+file);
		return false;
	}

	// We need to lock now //
	ObjectLock guard(*this);

	// temporary object data stores //
	vector<BaseGuiObject*> TempOs;

	// reserve space //
	TempOs.reserve(data.size());


	for(size_t i = 0; i < data.size(); i++){
		// check what type the object is //
		if(data[i]->TName == L"GuiCollection" || data[i]->TName == L"Collection"){

			if(!GuiCollection::LoadCollection(this, *data[i], LoadingView)){

				// report error //
				Logger::Get()->Error(L"GuiManager: ExecuteGuiScript: failed to load collection, named "+data[i]->Name);
				continue;
			}
			// delete rest of the object //
			goto guiprocessguifileloopdeleteprocessedobject;
		}
		if(data[i]->TName == L"GuiObject"){

			// try to load //
			if(!BaseGuiObject::LoadFromFileStructure(this, TempOs, *data[i], LoadingView)){

				// report error //
				Logger::Get()->Error(L"GuiManager: ExecuteGuiScript: failed to load GuiObject, named "+data[i]->Name);
				continue;
			}
			// delete rest of the object //
			goto guiprocessguifileloopdeleteprocessedobject;
		}

		Logger::Get()->Error(L"GuiManager: ExecuteGuiScript: Unrecognized type! typename: "+data[i]->TName);

guiprocessguifileloopdeleteprocessedobject:

		// delete current //
		data.erase(data.begin()+i);
		i--;
	}

	// Add the page //
	ThissViews.push_back(LoadingView);
	

	for(size_t i = 0; i < TempOs.size(); i++){

		// add to real objects //
		AddGuiObject(TempOs[i]);
	}

	// Set focus to the new View //
	ThissViews.back()->NotifyFocusUpdate(ThisWindow->GetWindow()->IsWindowFocused());

	return true;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Gui::GuiManager::SetMouseFile(const wstring &file){
	ObjectLock guard(*this);

	if(file == L"none"){

		// show default window cursor //
		ThisWindow->GetWindow()->SetHideCursor(false);
		if(file == L"none")
			return;
	}


	// hide window cursor //
	ThisWindow->GetWindow()->SetHideCursor(true);
}

DLLEXPORT void Leviathan::Gui::GuiManager::SetMouseFileVisibleState(bool state){
	ObjectLock guard(*this);
	// Set mouse drawing flag //
}
// ----------------- collection managing --------------------- //
void GuiManager::AddCollection(GuiCollection* add){
	ObjectLock guard(*this);
	Collections.push_back(add);
}
GuiCollection* Leviathan::Gui::GuiManager::GetCollection(const int &id, const wstring &name){
	ObjectLock guard(*this);
	// look for collection based on id or name //
	for(size_t i = 0; i < Collections.size(); i++){
		if(id >= 0){
			if(Collections[i]->GetID() != id){
				// no match //
				continue;
			}
		} else {
			// name should be specified, check for it //
			if(Collections[i]->GetName() != name){
				continue;
			}
		}

		// match
		return Collections[i];
	}

	return NULL;
}
// -------------------------------------- //
bool Leviathan::Gui::GuiManager::_CreateInternalOgreResources(Ogre::SceneManager* windowsscene){

	// Well we don't anymore need anything here since GuiView handles this //



	// Create a quad for mouse displaying //

	// Use RENDER_QUEUE_MAX to render on top of everything



	return true;
}

void Leviathan::Gui::GuiManager::_ReleaseOgreResources(){
	// We probably don't need to do anything //

	// The scene should handle deleting this //
	MouseQuad = NULL;

}

DLLEXPORT void Leviathan::Gui::GuiManager::SetAllowPaintStatus(bool canpaint /*= true*/){
	// Verify that Views can paint themselves //
	for(size_t i = 0; i < ThissViews.size(); i++){

		ThissViews[i]->SetAllowPaintStatus(canpaint);
	}
}



