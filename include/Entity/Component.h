#pragma once
#ifndef ENJON_COMPONENT_H
#define ENJON_COMPONENT_H 

#include "Entity/EntityDefines.h"
#include "Math/Transform.h"
#include "System/Types.h"
#include "Base/Object.h"

#include <assert.h>
#include <array>
#include <vector>
#include <bitset>
#include <type_traits>
#include <unordered_map> 

namespace Enjon 
{ 
	// Forward declaration
	class Entity;
	class EntityManager;
	class Component;

	class ComponentWrapperBase
	{
		public:
			virtual void Base() = 0;

			virtual bool HasEntity( const u32& entityID ) = 0;

			virtual Component* AddComponent( const u32& entityId, Component* component ) = 0;

	};

	template <typename T>
	class ComponentWrapper : public ComponentWrapperBase
	{
		friend Entity; 
		friend EntityManager;
		friend Component;

		public:
			void Base() override {}

			using ComponentPtrs = std::vector<T*>; 
			using ComponentMap = std::unordered_map<u32, T>;

			Component* AddComponent( const u32& entityId, Component* component )
			{
				mComponentMap[ entityId ] = *(T*)component;
				mComponentPtrs.push_back( &mComponentMap[ entityId ] );
				return &mComponentMap[ entityId ];
			} 

			virtual bool HasEntity( const u32& entityID ) override
			{
				return ( mComponentMap.find( entityID ) != mComponentMap.end( ) );
			}

		private:
			ComponentPtrs mComponentPtrs;
			ComponentMap mComponentMap;
	};

	class Component : public Enjon::Object
	{
		friend Entity;
		friend EntityManager; 

		public:

			/**
			* @brief Constructor
			*/
			Component(){}

			/**
			* @brief
			*/
			virtual void Update( f32 dT ) = 0;

			/**
			* @brief
			*/
			Entity* GetEntity() const;

			/**
			* @brief
			*/
			u32 GetID() const { return mComponentID; }

			/**
			* @brief
			*/
			EntityManager* GetEntityManager() const; 

		protected:

			template <typename T>
			void DestroyBase()
			{
				assert( mBase != nullptr );

				auto cWrapper = static_cast<ComponentWrapper<T>*>( mBase );
				auto cPtrList = &cWrapper->mComponentPtrs;
				auto cMap = &cWrapper->mComponentMap;

				// Get component
				auto compPtr = &cWrapper->mComponentMap[mEntityID];

				// Remove ptr from point list map
				cPtrList->erase( std::remove( cPtrList->begin(), cPtrList->end(), compPtr ), cPtrList->end() );	

				// Finally remove from map
				cMap->erase( mEntityID );
			}

			virtual void Destroy() = 0;

		private:
			void SetEntityManager( EntityManager* manager );
			void SetEntity( Entity* entity );
			void SetID( u32 id );
			void SetBase( ComponentWrapperBase* base );

		protected:
			Entity* mEntity = nullptr;
			EntityManager* mManager = nullptr;

			u32 mEntityID; 
			u32 mComponentID;

		private:
			ComponentWrapperBase* mBase = nullptr;
	}; 

	using ComponentID = u32;

	namespace Internal
	{
		inline ComponentID GetUniqueComponentID() noexcept
		{
			static ComponentID lastID{ 0u };
			return ++lastID;
		}
	}

	template <typename T>
	inline ComponentID GetComponentType() noexcept
	{
		static_assert( std::is_base_of<Component, T>::value, "Component:: T must inherit from Component." );	
		return Object::GetTypeId< T >( ); 
	}

	typedef std::bitset<static_cast<u32>( MAX_COMPONENTS )> ComponentBitset;

	template <typename T>
	ComponentBitset GetComponentBitMask() 
	{ 
		ComponentBitset BitSet;
		BitSet.set( GetComponentType<T>() );
		return BitSet;
	}

	ComponentBitset GetComponentBitMask( u32 type );
}

#endif
