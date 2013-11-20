#ifndef LEVIATHAN_ENTITY_PROP
#define LEVIATHAN_ENTITY_PROP
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Entities\Bases\BaseObject.h"
#include "Entities\Bases\BaseRenderable.h"
#include "Entities\Bases\BasePositionable.h"
#include "Entities\Bases\BaseScalable.h"
#include "..\Bases\BasePhysicsObject.h"
#include "..\Bases\BaseContraintable.h"
#include "ObjectFiles\ObjectFileProcessor.h"
#include "FileSystem.h"
#include "..\Bases\BaseParentable.h"

namespace Leviathan{ namespace Entity{
	

	class Prop : virtual public BaseObject, public BaseRenderable, public BaseContraintable, public BaseParentable{
	public:
		DLLEXPORT Prop(bool hidden, GameWorld* world);
		DLLEXPORT virtual ~Prop();
		
		DLLEXPORT bool Init(const wstring &modelfile);
		DLLEXPORT virtual void Release();

		static void PropPhysicsMovedEvent(const NewtonBody* const body, const dFloat* const matrix, int threadIndex);

		DLLEXPORT virtual bool SendCustomMessage(int entitycustommessagetype, void* dataptr);

	protected:
		// for setting new values to graphical object and physical object //
		void _UpdatePhysicsObjectLocation();
		virtual void _OnHiddenStateUpdated();
		// ------------------------------------ //
		Ogre::Entity* GraphicalObject;
		Ogre::SceneNode* ObjectsNode;
	};

}}
#endif