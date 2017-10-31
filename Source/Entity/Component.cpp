#include "Entity/Component.h"
#include "Entity/Entity.h"
#include "Entity/EntityManager.h"

namespace Enjon 
{
	Entity* Component::GetEntity() const
	{
		return mEntity;
	}

	void Component::SetEntity( Entity* entity )
	{
		mEntity = entity;
	}

	void Component::SetID( u32 id )
	{
		mID = id;
	}

	void Component::SetBase( ComponentWrapperBase* base )
	{
		mBase = base;
	} 

	ComponentBitset GetComponentBitMask( u32 type )
	{
		ComponentBitset BitSet;
		BitSet.set(type);
		return BitSet;
	}
}