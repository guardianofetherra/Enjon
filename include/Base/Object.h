// @file Object.h
// Copyright 2016-2017 John Jackson. All Rights Reserved.
#pragma once
#ifndef ENJON_OBJECT_H
#define ENJON_OBJECT_H 

#include "MetaClass.h"
#include "System/Types.h"
#include "Engine.h"
#include "Defines.h"

#include <limits>
#include <assert.h>
#include <functional>

#define ENJON_CLASS_BODY( type )																	\
	friend Enjon::Object;																			\
	public:																							\
		virtual u32 GetTypeId() const override { return Enjon::Object::GetTypeId< type >(); }		\
		virtual const char* GetClassName() const override { return #type; }							\
		virtual const Enjon::MetaClass* Class( ) override\
		{\
			Enjon::MetaClassRegistry* mr = const_cast< Enjon::MetaClassRegistry* >( Enjon::Engine::GetInstance()->GetMetaClassRegistry( ) );\
			const Enjon::MetaClass* cls = mr->Get< type >( );\
			if ( !cls )\
			{\
				cls = mr->RegisterMetaClass< type >( );\
			}\
			return cls;\
		}

#define ENJON_ARRAY(...)
#define ENJON_PROPERTY(...)
#define ENJON_FUNCTION(...)
#define ENJON_CLASS(...)

namespace Enjon
{
	enum class MetaPropertyType
	{
		Object,
		Bool,
		ColorRGBA16,
		F32,
		F64,
		U8,
		U16,
		U32,
		U64,
		S8,
		S16,
		S32,
		S64,
		String,
		Array,
		Vec2,
		Vec3,
		Vec4,
		Mat4,
		Quat,
		Enum,
		UUID,
		Transform,
		AssetHandle,
		EntityHandle
	};

	enum MetaPropertyFlags : u32
	{
		None = 0x00,
		IsPointer = 0x01,
		IsDoublePointer = 0x02
	};

	inline MetaPropertyFlags operator|( MetaPropertyFlags a, MetaPropertyFlags b )
	{
		return static_cast< MetaPropertyFlags >( static_cast< u32 >( a ) | static_cast< u32 >( b ) );
	}

	inline MetaPropertyFlags operator&( MetaPropertyFlags a, MetaPropertyFlags b )
	{
		return static_cast< MetaPropertyFlags >( static_cast< u32 >( a ) & static_cast< u32 >( b ) );
	}

	inline MetaPropertyFlags operator^( MetaPropertyFlags a, MetaPropertyFlags b )
	{
		return static_cast< MetaPropertyFlags >( static_cast< u32 >( a ) ^ static_cast< u32 >( b ) );
	}

	inline void operator^=( MetaPropertyFlags& a, MetaPropertyFlags b )
	{
		a = a ^ b;
	}

	inline void operator|=( MetaPropertyFlags& a, MetaPropertyFlags b )
	{
		a = a | b;
	}

	inline void operator&=( MetaPropertyFlags& a, MetaPropertyFlags b )
	{
		a = a & b;
	} 

	// Forward Declarations
	class MetaFunction;
	class MetaProperty;
	class MetaClass;
	class Object;
	
	// Don't really like this, but, ya know... wha ya gon' do?
	struct MetaPropertyTraits
	{
		MetaPropertyTraits( bool isEditable = false, f32 uiMin = 0.0f, f32 uiMax = 0.0f )
			: mIsEditable( isEditable ), mUIMin( uiMin ), mUIMax( uiMax )
		{ 
		}

		~MetaPropertyTraits( )
		{ 
		}

		/*
		* @brief
		*/
		f32 GetUIMax( ) const;
		
		/*
		* @brief
		*/
		f32 GetUIMin( ) const; 

		/*
		* @brief
		*/
		bool UseSlider( ) const;

		bool mIsEditable = false;
		f32 mUIMin = 0.0f;
		f32 mUIMax = 0.0f;
	};

	class MetaProperty
	{
		friend MetaClass;
		friend Object;

		public:
			/*
			* @brief
			*/
			MetaProperty( ) = default; 

			/*
			* @brief
			*/
			MetaProperty( MetaPropertyType type, const std::string& name, u32 offset, u32 propIndex, MetaPropertyTraits traits )
				: mType( type ), mName( name ), mOffset( offset ), mIndex( propIndex ), mTraits( traits )
			{
			}

			/*
			* @brief
			*/
			~MetaProperty( )
			{
			}
		
			/*
			* @brief
			*/
			bool HasFlags( MetaPropertyFlags flags );
			
			/*
			* @brief
			*/
			bool IsEditable( ) const;

			/*
			* @brief
			*/
			std::string GetName( );

			/*
			* @brief
			*/
			MetaPropertyType GetType( );

			/*
			* @brief
			*/
			MetaPropertyTraits GetTraits( ) const; 

			/*
			* @brief
			*/
			u32 GetOffset( )
			{
				return mOffset;
			}

			/*
			* @brief
			*/
			u32 GetIndex( )
			{
				return mIndex;
			}

		protected:
			MetaPropertyType mType;
			std::string mName;
			u32 mOffset;
			u32 mIndex;
			MetaPropertyTraits mTraits;
	};

	class MetaPropertyTemplateBase : public MetaProperty
	{
		public:
			virtual const MetaClass* GetClassOfTemplatedArgument( ) = 0;
	};

	template <typename T>
	class MetaPropertyAssetHandle : public MetaPropertyTemplateBase
	{ 
		public: 
			MetaPropertyAssetHandle( MetaPropertyType type, const std::string& name, u32 offset, u32 propIndex, MetaPropertyTraits traits )
			{
				static_assert( std::is_base_of<Object, T>::value, "Invoke() - T must inherit from Object." );

				mType = type;
				mName = name;
				mOffset = offset;
				mIndex = propIndex;
				mTraits = traits;
			}

			virtual const MetaClass* GetClassOfTemplatedArgument() override
			{
				return Object::GetClass<T>();	
			}

			T mClass;
	};

#define META_FUNCTION_IMPL( )\
	friend MetaClass;\
	friend Object;\
	virtual void Base( ) override\
	{\
	} 

	class MetaFunction
	{
		friend MetaClass;
		friend Object;

		public:
			MetaFunction( )
			{
			}

			~MetaFunction( )
			{ 
			}

			template < typename RetVal, typename T, typename... Args >
			RetVal Invoke( T* obj, Args&&... args )
			{
				static_assert( std::is_base_of<Object, T>::value, "Invoke() - T must inherit from Object." );
				return static_cast< MetaFunctionImpl< T, RetVal, Args&&... >* >( this )->mFunc( obj, std::forward<Args>( args )... );
			}

			const Enjon::String& GetName( ) const
			{
				return mName;
			}

		protected:
			virtual void Base( ) = 0;

		protected:
			Enjon::String mName = "";
	};

	template < typename T, typename RetVal, typename... Args >
	struct MetaFunctionImpl : public MetaFunction
	{ 
		META_FUNCTION_IMPL( )

		MetaFunctionImpl( std::function< RetVal( T*, Args&&... ) > function, const Enjon::String& name )
			: mFunc( function )
		{ 
			mName = name;
		}

		~MetaFunctionImpl( )
		{ 
		}

		std::function< RetVal( T*, Args&&... ) > mFunc;
	};

	template < typename T >
	struct MetaFunctionImpl< T, void, void > : public MetaFunction
	{ 
		META_FUNCTION_IMPL( )
		
		MetaFunctionImpl( std::function< void( T* ) > function, const Enjon::String& name )
			: mFunc( func )
		{ 
			mName = name;
		}

		~MetaFunctionImpl( )
		{ 
		}
		std::function< void( T* ) > mFunc;
	};

	template < typename T, typename RetVal >
	struct MetaFunctionImpl< T, RetVal, void > : public MetaFunction
	{
		META_FUNCTION_IMPL( )

		MetaFunctionImpl( std::function< RetVal( T* ) > function, const Enjon::String& name )
			: mFunc( func )
		{ 
			mName = name;
		}

		std::function< RetVal( T* ) > mFunc;
	};

	typedef std::vector< MetaProperty* > PropertyTable;
	typedef std::unordered_map< Enjon::String, MetaFunction* > FunctionTable;

	class MetaClass
	{
		friend Object;

		public:

			/*
			* @brief
			*/
			MetaClass( ) = default;

			/*
			* @brief
			*/
			~MetaClass( )
			{
				// Delete all functions
				for ( auto& f : mFunctions )
				{
					delete f.second;
				}

				// Clear properties and functions
				mProperties.clear( );
				mFunctions.clear( );
			}

			u32 PropertyCount( ) const
			{
				return ( u32 )mProperties.size( );
			}

			u32 FunctionCount( )
			{
				return ( u32 )mFunctions.size( );
			} 

			bool PropertyExists( const u32& index )
			{
				return index < mPropertyCount;
			}

			bool FunctionExists( const Enjon::String& name )
			{
				return ( mFunctions.find( name ) != mFunctions.end( ) ); 
			}

			const MetaFunction* GetFunction( const Enjon::String& name )
			{
				if ( FunctionExists( name ) )
				{
					return mFunctions[ name ];
				}

				return nullptr;
			}

			usize GetPropertyCount( )
			{
				return mPropertyCount;
			}

			const MetaProperty* GetProperty( const u32& index )
			{
				if ( PropertyExists( index ) )
				{
					return mProperties.at( index );
				}

				return nullptr;
			} 

			bool HasProperty( const MetaProperty* prop )
			{
				return ( ( prop->mIndex < mPropertyCount ) && ( mProperties.at( prop->mIndex ) == prop ) ); 
			}

			template < typename T >
			void GetValue( const Object* obj, const MetaProperty* prop, T* value )
			{
				if ( HasProperty( prop ) )
				{
					void* member_ptr = ( ( ( u8* )&( *obj ) + prop->mOffset ) );
					*value = *( T* )member_ptr;
				}
			} 

			template < typename RetVal > 
			RetVal* GetValueAs( const Object* obj, const MetaProperty* prop )
			{
				if ( HasProperty( prop ) )
				{
					void* member_ptr = ( ( ( u8* )&( *obj ) + prop->mOffset ) );
					return ( RetVal* )member_ptr;
				}

				return nullptr;
			} 

			template < typename T >
			void SetValue( const Object* obj, const MetaProperty* prop, const T& value )
			{
				if ( HasProperty( prop ) )
				{
					void* member_ptr = ( ( ( u8* )&( *obj ) + prop->mOffset ) );
					*( T* )( member_ptr ) = value;
				}
			} 

			template < typename T >
			bool InstanceOf( )
			{
				MetaClassRegistry* mr = const_cast< MetaClassRegistry* >( Engine::GetInstance( )->GetMetaClassRegistry( ) );
				const MetaClass* cls = mr->Get< T >( );
				if ( !cls )
				{
					cls = mr->RegisterMetaClass< T >( );
				} 

				return ( cls && cls == this ); 
			} 

			// Method for getting type id from MetaClass instead of Object
			u32 GetTypeId( ) const 
			{
				return mTypeId;
			}

			String GetName( )
			{
				return mName;
			}

			PropertyTable& GetProperties( ) { return mProperties; }

		protected:
			PropertyTable mProperties;
			FunctionTable mFunctions;
			u32 mPropertyCount;
			u32 mFunctionCount;
			u32 mTypeId;
			String mName;
	};

	class Object;
	class MetaClassRegistry
	{
		public:
			MetaClassRegistry( )
			{
			}

			/*
			* @brief
			*/
			~MetaClassRegistry( )
			{
				// Delete all meta classes from registry
				for ( auto& c : mRegistry )
				{
					delete c.second;
				}

				mRegistry.clear( );
			}

			template <typename T>
			MetaClass* RegisterMetaClass( )
			{
				// Must inherit from object to be able to registered
				static_assert( std::is_base_of<Object, T>::value, "MetaClass::RegisterMetaClass() - T must inherit from Object." );

				// Get id of object
				u32 id = Object::GetTypeId<T>( );

				// If available, then return
				if ( HasMetaClass< T >( ) )
				{
					return mRegistry[ id ];
				}

				// Otherwise construct it and return
				MetaClass* cls = Object::ConstructMetaClass< T >( );

				mRegistry[ id ] = cls;
				mRegistryByClassName[cls->GetName()] = cls;
				return cls;
			}

			/*
				MetaClass* cls = Object::ConstructMetaClassFromString(classString);
			*/

			template < typename T >
			bool HasMetaClass( )
			{
				return ( mRegistry.find( Object::GetTypeId< T >( ) ) != mRegistry.end( ) );
			}

			bool HasMetaClass( const String& className )
			{
				return ( mRegistryByClassName.find( className ) != mRegistryByClassName.end( ) );
			}

			template < typename T >
			const MetaClass* Get( )
			{
				return HasMetaClass< T >( ) ? mRegistry[ Object::GetTypeId< T >( ) ] : nullptr;
			}

			const MetaClass* GetClassByName( const String& className )
			{
				if ( HasMetaClass( className ) )
				{
					return mRegistryByClassName[className];
				}

				return nullptr;
			}

		private:
			std::unordered_map< u32, MetaClass* > mRegistry; 
			std::unordered_map< String, MetaClass* > mRegistryByClassName;
	};

	// Max allowed type id by engine
	const u32 EnjonMaxTypeId = std::numeric_limits<u32>::max( ) - 1;

	// Base model for all Enjon classes that participate in reflection
	class Object
	{
		friend MetaClassRegistry;
		friend Engine;

		public:

			/**
			*@brief
			*/
			Object( ) {}

			/**
			*@brief
			*/
			~Object( ) {}

			/**
			*@brief
			*/
			virtual const MetaClass* Class( ) = 0;

			/**
			*@brief
			*/
			virtual u32 GetTypeId( ) const = 0;

			/**
			*@brief
			*/
			virtual const char* GetClassName( ) const = 0; 

			/**
			*@brief
			*/
			template <typename T>
			T* Cast( )
			{
				static_assert( std::is_base_of<Object, T>::value, "Object::Cast() - T must inherit from Enjon Object." );

				return static_cast<T*>( this );
			}

			/**
			*@brief
			*/
			template <typename T>
			static u32 GetTypeId( ) noexcept
			{
				static_assert( std::is_base_of<Object, T>::value, "Object::GetTypeId() - T must inherit from Enjon Object." );

				static u32 typeId { GetUniqueTypeId( ) }; 
				return typeId;
			}

			template <typename T>
			bool InstanceOf( )
			{
				static_assert( std::is_base_of<Object, T>::value, "Object::GetTypeId() - T must inherit from Enjon Object." ); 
				return ( mTypeId == Object::GetTypeId< T >( ) );
			}

			/**
			*@brief
			*/
			bool TypeValid( ) const; 

			template <typename T>
			static const MetaClass* GetClass( )
			{ 
				MetaClassRegistry* mr = const_cast< MetaClassRegistry* >( Engine::GetInstance()->GetMetaClassRegistry( ) );
				const MetaClass* cls = mr->Get< T >( );
				if ( !cls )
				{
					cls = mr->RegisterMetaClass< T >( );
				}
				return cls;
			}

			/**
			*@brief Could return null!
			*/
			static const MetaClass* GetClass( const String& className )
			{
				MetaClassRegistry* mr = const_cast<MetaClassRegistry*> ( Engine::GetInstance( )->GetMetaClassRegistry( ) );
				return mr->GetClassByName( className );
			}

		protected:

			// Default to u32 max; If id set to this, then is not set by engine and invalid
			u32 mTypeId = 0;

		protected:
			template <typename T>
			static MetaClass* ConstructMetaClass( );

			static void BindMetaClasses( ); 

			template <typename T>
			static void RegisterMetaClass( )
			{
				MetaClassRegistry* mr = const_cast< MetaClassRegistry* >( Engine::GetInstance()->GetMetaClassRegistry( ) );
				mr->RegisterMetaClass< T >( );
			}

		private:

			/**
			*@brief
			*/
			static u32 GetUniqueTypeId( ) noexcept
			{
				static u32 lastId { 1 };
				return lastId++;
			} 
	};
}


#endif

