#include "Entity/EntityManager.h"
#include "Entity/Component.h"

#include <array>
#include <vector>
#include <assert.h>
#include <algorithm>

#include "ImGui/ImGuizmo.h"

#include <fmt/printf.h> 
#include <stdio.h>

namespace Enjon {

	//---------------------------------------------------------------
	Entity::Entity()
	: mID(MAX_ENTITIES), 
	  mState(EntityState::INACTIVE), 
	  mComponentMask(Enjon::ComponentBitset(0)),
	  mManager(nullptr),
	  mWorldTransformDirty(true)
	{
	}

	//---------------------------------------------------------------
	Entity::Entity(EntityManager* manager)
	: mID(MAX_ENTITIES), 
	  mState(EntityState::INACTIVE), 
	  mComponentMask(Enjon::ComponentBitset(0)),
	  mManager(manager),
	  mWorldTransformDirty(true)
	{
	}

	// TODO(John): Take care of removing all attached components here
	//---------------------------------------------------------------
	Entity::~Entity()
	{
		mManager->Destroy(this);		
	}

	//---------------------------------------------------------------
	void Entity::Reset()
	{
		assert(mManager != nullptr);

		// Remove from parent hierarchy list if in it
		if (!HasParent()) 
		{
			// Goes through and adds all children to hierarchy
			mManager->RemoveFromParentHierarchyList(this, true);

		}
		// Otherwise remove parent 
		else
		{
			RemoveParent(true, false);
		}

		// Remove all children and add to parent hierarchy list
		for (auto& c : mChildren)
		{
			c->RemoveParent(false, true);
		}

		mID = MAX_ENTITIES;
		mState = EntityState::INACTIVE;
		mWorldTransformDirty = true;
		mComponentMask = Enjon::ComponentBitset(0);
		mManager = nullptr;
		mComponents.clear();
		mChildren.clear();
	}

	//---------------------------------------------------------------
	void Entity::SetID(u32 id)
	{
		mID = id;
	}

	//---------------------------------------------------------------
	void Entity::SetLocalTransform(Transform& transform)
	{
		mLocalTransform = transform;
		mWorldTransformDirty = true;
	}

	//---------------------------------------------------------------
	Transform Entity::GetLocalTransform()
	{
		return mLocalTransform;
	}

	//---------------------------------------------------------------
	Transform Entity::GetWorldTransform()
	{
		// Calculate world transform
		CalculateWorldTransform();

		// Return world transform
		return mWorldTransform;
	}

	//---------------------------------------------------------------
	void Entity::CalculateLocalTransform( ) 
	{
		// RelScale = Scale / ParentScale 
		// RelRot	= Conjugate(ParentRot) * Rot
		// Trans	= 1/ParentScale * [Conjugate(ParentRot) * (Position - ParentPosition)];
		if ( HasParent( ) )
		{ 
			Transform parentTransform = mParent->GetWorldTransform( );
			Enjon::Quaternion parentConjugate = parentTransform.Rotation.Conjugate( ); 

			Vec3 relativeScale		= parentConjugate * ( mLocalTransform.Scale / parentTransform.Scale );
			Quaternion relativeRot	= parentConjugate * mLocalTransform.Rotation;
			Vec3 relativePos		= ( parentConjugate * ( mLocalTransform.Position - parentTransform.Position ) ) / parentTransform.Scale;

			mLocalTransform = Transform( relativePos, relativeRot, relativeScale );
		}
	}

	//---------------------------------------------------------------
	void Entity::CalculateWorldTransform()
	{ 
		if ( !HasParent() ) 
		{
			mWorldTransform = mLocalTransform;
			return;
		} 
		
		// Get parent transform recursively
		Transform parent = mParent->GetWorldTransform( ); 

		/*
		Mat4 world(1.0f);
		world *= Mat4::Translate(mLocalTransform.Position);
		world *= QuaternionToMat4(mLocalTransform.Rotation);
		world *= Mat4::Scale(mLocalTransform.Scale);
		
		Mat4 parentMat(1.0f);
		parentMat *= Mat4::Translate(parent.Position);
		parentMat *= QuaternionToMat4(parent.Rotation);
		parentMat *= Mat4::Scale(parent.Scale);

		Vec3 s = parent.Scale;
		Quaternion q = parent.Rotation.Normalize();
		Vec3 p = parent.Position;
		Vec3 t = mLocalTransform.Position; 

		f32 a = q.y * q.y + q.z * q.z;
		f32 b = q.x * q.y - q.w * q.z;
		f32 c = q.x * q.z + q.w * q.y;
		f32 d = q.x * q.y + q.w * q.z;
		f32 e = q.x * q.x + q.z * q.z;
		f32 f = q.y * q.z - q.w * q.x;0
		f32 g = q.x * q.z - q.w * q.y;
		f32 h = q.y * q.y + q.w * q.x;
		f32 i = q.x * q.x + q.y * q.y;

		f32 x = s.x * ( 1.0f - 2 * a )	* t.x +
				s.y * ( 2.0f * b )		* t.y +
				s.z * ( 2.0f * c )		* t.z +
				p.x;

		f32 y = s.x * ( 2.0f * d )			* t.x + 
				s.y * ( 1.0f - 2.0f * e )	* t.y + 
				s.z * ( 2.0f * f )			* t.z + 
				p.y;

		f32 z = s.x * ( 2.0f * g )			* t.x + 
				s.y * ( 2.0f * h )			* t.y + 
				s.z * ( 1.0f - 2.0f * i )	* t.z + 
				p.z; 


		//Mat4 worldMat = parentMat * world; 
		

		//Vec3 worldPos = Vec3( worldMat.columns[3].x, worldMat.columns[3].y, worldMat.columns[3].z );
		Vec3 worldPos = Vec3( x, y, z );
		Vec3 worldScale = parent.Scale * mLocalTransform.Scale; 
		Quaternion worldRot = parent.Rotation * mLocalTransform.Rotation; 

		// Not correct...
		//Vec3 worldPos			= ( parent.Rotation.Rotate( mLocalTransform.Position ) + parent.Position ) * parent.Scale;
		*/

		Enjon::Vec3 worldPos = parent.Position + parent.Rotation.Rotate( parent.Scale * mLocalTransform.Position );
		//Enjon::Vec3 worldPos = parent.Position + parent.Rotation * ( parent.Scale * mLocalTransform.Position );
		Enjon::Vec3 worldScale = parent.Scale * ( parent.Rotation * mLocalTransform.Scale );
		Enjon::Quaternion worldRot = parent.Rotation * mLocalTransform.Rotation; 

		mWorldTransform = Transform( worldPos, worldRot, worldScale );
			
		mWorldTransformDirty = false;
	}

	//---------------------------------------------------------------
	void Entity::SetPosition(Vec3& position)
	{
		mLocalTransform.SetPosition(position);
		mWorldTransformDirty = true;
	}

	//---------------------------------------------------------------
	void Entity::SetScale(f32 scale)
	{
		SetScale(v3(scale));
	}

	//---------------------------------------------------------------
	void Entity::SetScale(Vec3& scale)
	{
		mLocalTransform.SetScale(scale);
		mWorldTransformDirty = true;
	}

	//---------------------------------------------------------------
	void Entity::SetRotation(Quaternion& rotation)
	{
		mLocalTransform.SetRotation(rotation);
		mWorldTransformDirty = true;
	}

	//---------------------------------------------------------------
	Vec3 Entity::GetLocalPosition()
	{
		return mLocalTransform.GetPosition();
	}

	//---------------------------------------------------------------
	Vec3 Entity::GetLocalScale()
	{
		return mLocalTransform.GetScale();
	}

	//---------------------------------------------------------------
	Quaternion Entity::GetLocalRotation()
	{
		return mLocalTransform.GetRotation();
	}

	//---------------------------------------------------------------
	Vec3 Entity::GetWorldPosition()
	{
		if (mWorldTransformDirty) CalculateWorldTransform();
		return mWorldTransform.GetPosition();
	}

	//---------------------------------------------------------------
	Vec3 Entity::GetWorldScale()
	{
		if (mWorldTransformDirty) CalculateWorldTransform();
		return mWorldTransform.GetScale();
	}

	//---------------------------------------------------------------
	Quaternion Entity::GetWorldRotation()
	{
		if (mWorldTransformDirty) CalculateWorldTransform();
		return mWorldTransform.GetRotation();
	}

	//-----------------------------------------
	Entity* Entity::AddChild(Entity* child)
	{
		// Set parent to this
		child->SetParent(this);

		// Make sure child doesn't exist in vector before pushing back
		auto query = std::find(mChildren.begin(), mChildren.end(), child);
		if (query == mChildren.end())
		{
			// Add child to children list
			mChildren.push_back(child);

			// Calculate its world transform with respect to parent
			child->CalculateWorldTransform();
		}
		else
		{
			// Log a warning message here
		}

		return this;
	}

	//-----------------------------------------
	void Entity::DetachChild(Entity* child, b8 removeFromList, b8 addToHeirarchy)
	{
		// Make sure child exists
		assert(child != nullptr);
		assert(mManager != nullptr);

		// Find and erase
		if (removeFromList) 
		{
			mChildren.erase(std::remove(mChildren.begin(), mChildren.end(), child), mChildren.end());

		}

		if (addToHeirarchy)
		{
			// Add to child to parent hierarchy list now that it's a free node
			mManager->AddToParentHierarchy(child);
		}

		// Recalculate world transform of child
		child->CalculateWorldTransform();

		// Set parent to nullptr
		child->mParent = nullptr;

	}

	

	//-----------------------------------------
	void Entity::SetParent(Entity* parent)
	{
		// Make sure this child doesn't have a parent
		assert(parent != nullptr);
		assert(mParent == nullptr);
		assert(mManager != nullptr);

		// Set parent to this
		mParent = parent; 
		
		// Calculate local transform
		CalculateLocalTransform( );

		// Calculate world transform
		CalculateWorldTransform( ); 

		// Now that the parent is set, remove it from hierarchy list
		mManager->RemoveFromParentHierarchyList(this);
	}

	//-----------------------------------------
	Entity* Entity::RemoveParent(b8 removeFromList, b8 addToHierarchy)
	{
		// Asset parent exists
		assert(mParent != nullptr);

		// Remove child from parent
		mParent->DetachChild(this, removeFromList, addToHierarchy);

		// Capture pointer
		Entity* retNode = mParent;

		// Set parent to nullptr
		mParent = nullptr;

		return retNode;
	}

	//---------------------------------------------------------------
	b8 Entity::HasChildren()
	{
		return (mChildren.size() > 0);
	}

	//---------------------------------------------------------------
	b8 Entity::HasParent()
	{
		return (mParent != nullptr);
	}

	//---------------------------------------------------------------
	b8 Entity::IsValid()
	{
		return (mState != EntityState::INACTIVE);
	}

	//---------------------------------------------------------------
	void Entity::SetAllChildWorldTransformsDirty()
	{
		for (auto& c : mChildren)
		{
			// Set dirty to true
			c->mWorldTransformDirty = true;

			// Iterate through child's children to set their state dirty as well
			c->SetAllChildWorldTransformsDirty();
		}
	}

	void Entity::PropagateTransform(f32 dt)
	{
		// Calculate world transform
		mWorldTransform = mLocalTransform;
		if (HasParent()) mWorldTransform *= mParent->mWorldTransform;

		// Iterate through children and propagate down
		for (auto& c : mChildren)
		{
			c->PropagateTransform(dt);
		}

		UpdateComponentTransforms(dt);
	}

	//---------------------------------------------------------------
	void Entity::UpdateAllChildTransforms()
	{
		// Maybe this should just be to set their flags to dirty?
		for (auto& c : mChildren)
		{
			c->mWorldTransformDirty = true;
			c->CalculateWorldTransform();
		}
	}

	//---------------------------------------------------------------
	void Entity::UpdateComponentTransforms(f32 dt)
	{
		if ( mWorldTransformDirty )
		{
			CalculateWorldTransform( );
		}

		for (auto& c : mComponents)
		{
			c->Update(dt);
		}
	}

	//---------------------------------------------------------------
	EntityManager::EntityManager()
	{
		for (auto i = 0; i < mComponents.size(); i++)
		{
			mComponents.at(i) = nullptr;
		}

		mNextAvailableID = 0;
		mEntities = new std::array<Entity, MAX_ENTITIES>;
	}

	//---------------------------------------------------------------
	EntityManager::~EntityManager()
	{
		// Detach all components from entities
		for (u32 i = 0; i < MAX_ENTITIES; ++i)
		{
			Destroy(&mEntities->at(i));	
		}
		delete[] mEntities;

		// Deallocate all components
		for (u32 i = 0; i < mComponents.size(); ++i)
		{
			delete mComponents.at(i);
			mComponents.at(i) = nullptr;
		}
	}

	//---------------------------------------------------------------
	u32 EntityManager::FindNextAvailableID()
	{
		// Iterate from current available id to MAX_ENTITIES
		for (u32 i = mNextAvailableID; i < MAX_ENTITIES; ++i)
		{
			if (mEntities->at(i).mState == EntityState::INACTIVE)
			{
				mNextAvailableID = i;
				return mNextAvailableID;
			}
		}

		// Iterate from 0 to mNextAvailableID
		for (u32 i = 0; i < mNextAvailableID; ++i)
		{
			if (mEntities->at(i).mState == EntityState::INACTIVE)
			{
				mNextAvailableID = i;
				return mNextAvailableID;
			}
		}

		// Other wise return MAX_ENTITIES, since there are no entity slots left
		return MAX_ENTITIES;
	}

	//---------------------------------------------------------------
	Entity* EntityManager::Allocate()
	{
		// Grab next available id and assert that it's valid
		u32 id = FindNextAvailableID();
		assert(id < MAX_ENTITIES);

		// Find entity in array and set values
		Entity* entity = &mEntities->at(id);
		entity->mID = id;				
		entity->mState = EntityState::ACTIVE; 
		entity->mManager = this;

		// Push back live entity into active entity vector
		mActiveEntities.push_back(entity);

		// Push back live entity into parent hierarchy list
		AddToParentHierarchy(entity);

		// Return entity handle
		return entity;
	}

	//---------------------------------------------------------------
	void EntityManager::Destroy(Entity* entity)
	{
		// Assert that entity isn't already null
		assert(entity != nullptr);

		// Push for deferred removal from active entities
		mMarkedForDestruction.push_back(entity);
	}

	//--------------------------------------------------------------
	void EntityManager::Cleanup()
	{
		// Move through dirty list and remove from active entities
		for (auto& e : mMarkedForDestruction)
		{
			// Destroy all components
			for (auto& c : e->mComponents)
			{
				c->Destroy();
			}

			// Reset entity
			e->Reset();

			// Remove from active entities
			mActiveEntities.erase(std::remove(mActiveEntities.begin(), mActiveEntities.end(), e), mActiveEntities.end());
		}

		mMarkedForDestruction.clear();
	}

	//--------------------------------------------------------------
	void EntityManager::Update(f32 dt)
	{
		// Clean any entities that were marked for destruction
		Cleanup();
	} 

	//--------------------------------------------------------------
	void EntityManager::LateUpdate(f32 dt)
	{
		// Clean any entities that were marked for destruction
		UpdateAllActiveTransforms(dt);
	}

	//--------------------------------------------------------------
	b8 EntityManager::CompareEntityIDs(const Entity* a, const Entity* b)
	{
		return a->mID < b->mID;	
	}

	//--------------------------------------------------------------
	void EntityManager::AddToParentHierarchy(Entity* entity)
	{
		if (entity == nullptr) return;

		// Add to parent hierarchy list
		// Does this need to check if entity already exists?

		if (std::find(mEntityParentHierarchy.begin(), mEntityParentHierarchy.end(), entity) == mEntityParentHierarchy.end() && entity->mID < MAX_ENTITIES)
		{
			mEntityParentHierarchy.push_back(entity);
		}

		// Sort list
		std::stable_sort(mEntityParentHierarchy.begin(), mEntityParentHierarchy.end(), CompareEntityIDs);	
	}

	//--------------------------------------------------------------
	void EntityManager::RemoveFromParentHierarchyList(Entity* entity, b8 toBeDestroyed)
	{
		// Some checks need to be made here
		if (entity == nullptr) return;

		// Find and erase from hierarchy
		// TODO(): Make sure this uses binary search for speed up
		mEntityParentHierarchy.erase(std::remove(mEntityParentHierarchy.begin(), mEntityParentHierarchy.end(), entity), mEntityParentHierarchy.end());

		// Check if entity is to be destroyed this frame or not
		// If not to be destroyed, then this entity is being attached to another as a child
		// and its children will come along with it
		if (toBeDestroyed)
		{
			// If children, add them to parent hierarchy as free nodes
			if (entity->HasChildren())
			{
				for (auto& c : entity->mChildren)
				{
					AddToParentHierarchy(c);	
				}
			}
		}
	}

	//--------------------------------------------------------------
	void EntityManager::UpdateAllActiveTransforms(f32 dt)
	{
		// Iterate though parent hierarchy and calculate transforms
		for (auto& p : mEntityParentHierarchy)
		{
			p->PropagateTransform(dt);
		}
	}
}


















