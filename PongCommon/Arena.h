#ifndef PONG_ARENA
#define PONG_ARENA
// ------------------------------------ //
#ifndef PONGINCLUDES
#include "PongIncludes.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "PlayerSlot.h"
#include "Entities/Objects/TrailEmitter.h"


#define BASE_ARENASCALE		1.f	

namespace Pong{

	class BasePongParts;

	class Arena{
	public:
		Arena(shared_ptr<Leviathan::GameWorld> world);
		~Arena();
		// Generates an arena to the world //
		bool GenerateArena(BasePongParts* game, PlayerList &plys);

        //! Makes sure the trail object exists
        void VerifyTrail();

		void ServeBall();
		// Does what ever is needed to ditch old ball //
		void LetGoOfBall();

		inline shared_ptr<Leviathan::GameWorld> GetWorld(){
			return TargetWorld;
		}

        void RegisterBall(shared_ptr<Leviathan::BaseObject> ball){

            Ball = ball;
        }

		string GetMaterialNameForPlayerColour(const Float4 &colour);

		void ColourTheBallTrail(const Float4 &colour);

		inline shared_ptr<Leviathan::BaseObject> GetBallPtr(){
			return Ball;
		}

		// Checks based on generated arena if ball intersects (or could) with a paddle area //
		bool IsBallInPaddleArea();

	private:

		void _ClearPointers();
		// ------------------------------------ //

		// the world to which the arena is generated //
		shared_ptr<Leviathan::GameWorld> TargetWorld;

		// Stored object pointers //

		// Arena bottom //
		shared_ptr<Leviathan::BaseObject> BottomBrush;

		// The ball trail object //
		shared_ptr<Leviathan::BaseObject> TrailKeeper;
		Leviathan::Entity::TrailEmitter* DirectTrail;

		// ball prop //
		shared_ptr<Leviathan::BaseObject> Ball;

		// Used to store already generated materials for paddles //
		std::map<Float4, string> ColourMaterialName;

	};

}
#endif
