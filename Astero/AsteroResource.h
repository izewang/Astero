//
//  AsteroResource.h
//  Astero
//
//  Created by Yuzhe Wang on 8/20/17.
//  Copyright © 2017 Yuzhe Wang. All rights reserved.
//

#ifndef AsteroResource_h
#define AsteroResource_h

#include <mutex> // resource_mutex
#include <memory> // shared_ptr
#include <unordered_map>
#include <string>
#include <atomic>
#include <set>
#include "AsteroDataStream.h"

namespace Astero {
	// Abstract class representing a loadable resource.
	// Resources can be unloaded when memory budge is under stress.
	class Resource {
	public:
		typedef unsigned long long int Handle;
		typedef std::mutex Mutex;

		class Listener {
		public:
			Listener() = default;
			virtual ~Listener() = default;
			
			// Called whenever events occured.
			virtual void preparingComplete(Resource *) {}
			virtual void loadingComplete(Resource *) {}
			virtual void unloadingComplete(Resource *) {}
		};
		
		enum LoadingState {
			LOADSTATE_UNLOADED,
			LOADSTATE_UNLOADING,
			LOADSTATE_LOADED,
			LOADSTATE_LOADING,
			LOADSTATE_PREPARED,
			LOADSTATE_PREPARING
		};
		
		Resource() = delete;
		Resource(Handle handle, const std::string & name, const std::string & group)
			: handle_(0), name_(name), group_(group), loading_state_(LOADSTATE_UNLOADED), size_(0) {}
		virtual ~Resource() = default;
		
		virtual void prepare() {
			// If resource is not unloaded or preparing, return.
			LoadingState old_state = loading_state_;
			if (old_state != LOADSTATE_UNLOADED && old_state != LOADSTATE_PREPARING)
				return;
			// If resource is not unloaded, check if resource finishs preparing and return.
			// If resource is unloaded, set state as PREPARING.
			LoadingState state_unloaded(LOADSTATE_UNLOADED);
			if (!loading_state_.compare_exchange_strong(state_unloaded, LOADSTATE_PREPARING)) {
				// Wait until other thread finish preparing.
				while (loading_state_ == LOADSTATE_PREPARING)
					Lock lock(mutex_);
				old_state = loading_state_;
				assert(old_state == LOADSTATE_PREPARED || old_state == LOADSTATE_LOADING || old_state == LOADSTATE_LOADED);
				return;
			}
			// Lock mutex.
			Lock lock(mutex_);
			// Prepare resource here.
			prepareImpl();
		}
		virtual void load() {
			LoadingState old_state = loading_state_;
			bool keepchecking = true;
			while (keepchecking) {
				// Quick check that avoid synchronization.
				// Do nothing if resource is loaded or unloading.
				if (old_state == LOADSTATE_LOADED || old_state == LOADSTATE_UNLOADING)
					return;
				// When other thread is preparing, wait until state changed, and update old_state.
				else if (old_state == LOADSTATE_PREPARING) {
					while (loading_state_ == LOADSTATE_PREPARING)
						Lock lock(mutex_);
					old_state = loading_state_;
					continue;
				}
				// when other thread is loading, wait and check if resource is loaded.
				else if (old_state == LOADSTATE_LOADING) {
					while (loading_state_ == LOADSTATE_LOADING)
						Lock lock(mutex_);
					assert(loading_state_ == LOADSTATE_LOADED);
					return;
				}
				// If state changed, update old_state and continue.
				// If state is not changed, set state as LOADING and leave loop.
				else if (!loading_state_.compare_exchange_strong(old_state, LOADSTATE_LOADING)) {
					old_state = loading_state_;
					continue;
				}
				keepchecking = false;
			}
			// Resource state is prepared or unloaded.
			// Prepare and load resource here.
			if (old_state == LOADSTATE_UNLOADED)
				prepareImpl();
			Lock lock(mutex_);
			preLoadImpl();
			loadImpl();
			postLoadImpl();
			calculateSize();
		}
		virtual void unload() {
			LoadingState old_state = loading_state_;
			// Only unload when loading state is LOADED or PREPARED.
			if (old_state != LOADSTATE_LOADED && old_state != LOADSTATE_PREPARED)
				return;
			// Do nothing when loading state changes.
			if (!loading_state_.compare_exchange_strong(old_state, LOADSTATE_UNLOADING))
				return;
			// Lock mutex.
			Lock lock(mutex_);
			if (old_state == LOADSTATE_PREPARED)
				unprepareImpl();
			else {
				preUnloadImpl();
				unloadImpl();
				postUnloadImpl();
			}
			// Set loading state as UNLOADED.
			loading_state_ = LOADSTATE_UNLOADED;
		}
		// Getters and setters.
		virtual size_t getSize() const {
			return size_;
		}
		virtual const std::string & getName() const {
			return name_;
		}
		virtual const std::string & getGroup() const {
			return group_;
		}
		virtual Handle getHandle() const {
			return handle_;
		}
		virtual LoadingState getLoadingState() const {
			return loading_state_;
		}
		virtual void addListener(Listener* listener) {
			Lock lock(mutex_);
			listener_set_.insert(listener);
		}
		virtual void removeListener(Listener* listener) {
			Lock lock(mutex_);
			listener_set_.erase(listener);
		}
		// Public mutex for lock.
		Mutex mutex_;
	protected:
		typedef std::set<Listener *> ListenerSet;
		typedef std::lock_guard<Mutex> Lock;
		
		// Numeric handle for efficient look up
		Handle handle_;
		// Unique name of the resource
		std::string name_;
		// The name of the group
		std::string group_;
		// Atomic loading state
		std::atomic<LoadingState> loading_state_;
		// The size of resource in bytes
		size_t size_;
		// Set of listeners
		ListenerSet listener_set_;
		Mutex listener_set_mutex_;
		
		// Internal hook functions
		virtual void preLoadImpl() {}
		virtual void postLoadImpl() {}
		virtual void preUnloadImpl() {}
		virtual void postUnloadImpl() {}
		virtual void prepareImpl() {}
		virtual void unprepareImpl() {}
		virtual void loadImpl() {}
		virtual void unloadImpl() {}
		// Calculate sizeof resource.
		virtual void calculateSize() {
			size_ = sizeof(*this);
		}
	};
	
	// ResourceManager is responsible for managing a pool of resources
	// of a particular type. It may stay within a defined memory budget,
	// and temporary unload some when budget is depleted.
	template <typename T>
	class Pool {
	public:
		typedef std::mutex Mutex;
		
		Pool() = default;
		virtual ~Pool() = default;
		
		virtual void push_front(const T &);
		virtual T pop_front();
		virtual void clear();
		virtual bool empty() const;
		Mutex mutex_;

	protected:
		
	};
	
	typedef std::shared_ptr<Resource> ResourcePtr;
	
	class ResourceManager {
	public:
		ResourceManager() = default;
		virtual ~ResourceManager() = default;
		
		virtual void setMemoryBudget(size_t bytes);
		virtual size_t getMemoryBudget(void) const;
		virtual size_t getMemoryUsage(void) const;
		
		virtual ResourcePtr createResource(const std::string & name);
		virtual ResourcePtr prepareResource(const std::string & name);
		virtual ResourcePtr loadResource(const std::string & name);
		virtual void unload(const std::string & name);
		virtual ResourcePtr reloadResource(const std::string & name);
		virtual void removeResource(ResourcePtr & resource_ptr);
		virtual void removeAll();
		
		class ResourcePool : public Pool<ResourcePtr> {
		public:
			ResourcePool(const std::string & name) : name_(name) { }
			~ResourcePool() = default;
			
			const std::string & getName() const {
				return name_;
			}
			void clear() override;
		protected:
			std::string name_;
		};
		
		std::mutex mutex;
	protected:
		typedef std::unordered_map<std::string, ResourcePtr> ResourceMap;
		typedef ResourceMap::iterator ResourceMapIterator;
		typedef std::unordered_map<std::string, ResourcePool*> ResourcePoolMap;
		
		ResourceMap mResources;
		ResourcePoolMap mResourcePoolMap;
	};
	
	class Archive;

	class ResourceGroupManager {
	public:
		ResourceGroupManager();
		virtual ~ResourceGroupManager();
		
		void createResourceGroup();
		void initializeResourceGroup(std::string &);
		void initializeAllResourceGroup();
		void prepareResourceGroup(std::string &);
		void loadResourceGroup();
		void unloadResourceGroup();
		void clearResourceGroup();
		void destroyResourceGroup();
		
		void addResourceLocation();
		void removeResourceLocation();
		void declareResource();
		void createResource();
		void deleteResource();
		void openResource();
		
		void addListener();
		void removeListener();
		
		void shutdown();
		
		DataStreamPtr openResource(const std::string & name, const std::string & group);
		
		static ResourceGroupManager & getSingleton();
		static ResourceGroupManager * getSingletonPtr();
		
		std::mutex mutex;
		
	protected:
		struct ResourceDeclaration {
		public:
			std::string name;
			std::string type;
		};
		
		struct ResourceLocation {
			Archive* archive;
			bool recursive;
		};
		
		typedef std::unordered_map<std::string, ResourceManager *> ResourceManagerMap;
		typedef std::list<ResourceDeclaration> ResourceDeclarationList;
		typedef std::list<ResourceLocation> ResourceLocationList;
		struct ResourceGroup {
			enum Status {
				UNINITIALIZED = 0,
				INITIALIZING = 1,
				INITIALIZED = 2,
				LOADING = 3,
				LOADED = 4
			};
			std::mutex mutex;
			std::string name;
			Status group_status;
			ResourceLocationList location_list;
		};
		ResourceManagerMap mResourceManagerMap;
		typedef std::unordered_map<std::string, ResourceGroup *> ResourceGroupMap;
		ResourceGroupMap mResourceGroupMap;
		
	};
} // namespace Astero

#endif // AsteroResource_h
