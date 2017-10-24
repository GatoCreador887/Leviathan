// Leviathan Game Engine
// Copyright (c) 2012-2017 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "EntityCommon.h"
#include "Common/ObjectPool.h"

#include "boost/pool/pool.hpp"

#include <array>

namespace Leviathan{

//! Number of states that are kept. Corresponds to time span of TICKSPEED * KEPT_STATES_COUNT
constexpr auto KEPT_STATES_COUNT = 5;

template<class StateT>
    class StateHolder;

template<class StateT>
class ObjectsComponentStates{
public:

    ObjectsComponentStates(){

        for(size_t i = 0; i < StoredStates.size(); ++i){

            StoredStates[i] = nullptr;
        }
    }

    //! \brief Returns the state with the highest tick number
    StateT* GetNewest() const{

        StateT* newest = nullptr;
        int newestTick = 0;

        for(StateT* state : StoredStates){

            if(state && state->TickNumber >= newestTick){

                newest = state;
                newestTick = state->TickNumber;
            }
        }

        return newest;
    }

    //! \brief Returns the state with the lowest tick number
    StateT* GetOldest() const{

        StateT* oldest = nullptr;
        int oldestTick = std::numeric_limits<int>::max();

        for(StateT* state : StoredStates){

            if(state && state->TickNumber <= oldestTick){

                oldest = state;
                oldestTick = state->TickNumber;
            }
        }

        return oldest;
    }

    //! \brief Returns the state matching the tick number or the
    //! closest tick that is higher than the tick number
    //! \todo How can the caller of this method detect when a state
    //! has been missed and should instead (maybe) wait a bit? Or
    //! should we just skip missed states and start interpolating from
    //! the later state
    StateT* GetMatchingOrNewer(int ticknumber) const{

        StateT* closest = nullptr;
        int closestBy = std::numeric_limits<int>::max();

        for(StateT* state : StoredStates){

            if(!state || state->TickNumber < ticknumber)
                continue;

            if(state->TickNumber == ticknumber){
                
                return state;
            }

            const auto difference = state->TickNumber - ticknumber;
            if(difference <= closestBy){

                closestBy = difference;
                closest = state;
            }
        }

        return closest;        
    }

    //! \brief Returns state matching tick number
    StateT* GetState(int ticknumber) const{

        for(StateT* state : StoredStates){

            if(state && state->TickNumber == ticknumber)
                return state;
        }

        return nullptr;
    }

    //! \brief Returns true if state is still valid
    bool IsStateValid(StateT* statetocheck) const{

        for(StateT* state : StoredStates){
            if(state == statetocheck)
                return true;
        }
        
        return false;
    }

    //! \brief Appends a new state removing old states if they don't fit anymore
    void Append(StateT* newstate, StateHolder<StateT> &stateowner){

        // Fill from the back and pop from the front if states don't fit //

        // This would only speed up the first added state, so maybe the performance
        // is better if we skip this check
        // if(StoredStates[KEPT_STATES_COUNT - 1] == nullptr){

        //     StoredStates[KEPT_STATES_COUNT - 1] = newstate;
        //     return;
        // }

        // First state will be popped off if it exists //
        if(StoredStates[0] != nullptr){

            stateowner._DestroyStateObject(StoredStates[0]);
        }

        for(size_t i = 0; i < KEPT_STATES_COUNT - 1; ++i){

            StoredStates[i] = StoredStates[i + 1];
        }

        StoredStates[KEPT_STATES_COUNT - 1] = newstate;
    }

    //! \brief Counts the filled number of state slots
    //! \returns A number in range [0, KEPT_STATES_COUNT]
    auto GetNumberOfStates() const{

        int count = 0;
        
        for(StateT* state : StoredStates){

            if(state)
                ++count;
        }
        
        return count;
    }
    
protected:
    std::array<StateT*, KEPT_STATES_COUNT> StoredStates;
};

//! \brief Holds state objects of type for quick access by ObjectID
//! \todo The GameWorld needs to notify this when ObjectID is deleted
template<class StateT>
class StateHolder{
    friend ObjectsComponentStates<StateT>;
public:

    StateHolder(){

    }
    
    ~StateHolder(){
        // Both of the pools are released here so the destructor of ObjectsComponentStates
        // doesn't need to notify us of the states it held
    }

    //! \brief Creates a new state for entity's component if it has changed
    //! \returns True if a new state was created
    template<class ComponentT>
        bool CreateStateIfChanged(ObjectID id, const ComponentT &component,
            int ticknumber)
    {
        ObjectsComponentStates<StateT>* entityStates = StateObjects.Find(id);

        if(!entityStates){

            entityStates = StateObjects.ConstructNew(id);
        }

        // Get latest state to compare current values against //
        StateT* latestState = entityStates->GetNewest();

        // If empty always create //
        if(!latestState){

            StateT* newState = _CreateNewState(ticknumber, component);
            entityStates->Append(newState, *this);
            return true;
        }

        // Check is the latest state still correct //
        if(latestState->DoesMatchState(component))
            return false;

        // Create a new state //
        StateT* newState = _CreateNewState(ticknumber, component);
        entityStates->Append(newState, *this);
        return true;        
    }

    //! \brief Returns the number of entities that have states
    auto GetNumberOfEntitiesWithStates() const{

        return StateObjects.GetObjectCount();
    }

    //! \brief Returns a pointer to entity's states if they exist
    ObjectsComponentStates<StateT> const* GetEntityStates(ObjectID id) const{

        return StateObjects.Find(id);
    }
    
protected:

    template<typename... Args>
    StateT* _CreateNewState(Args&&... args){
        
        return StatePool.ConstructNew(std::forward<Args>(args)...);
    }

    void _DestroyStateObject(StateT* state){

        StatePool.Destroy(state);
    }

private:
    
    //! Keeps track of states associated with an object
    ObjectPool<ObjectsComponentStates<StateT>, ObjectID> StateObjects;

    //! All of the created StateT objects are held by this pool and
    //! only referenced from StateObjects
    BasicPool<StateT> StatePool;
};

}

