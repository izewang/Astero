//
//  AsteroAllocator.tpp
//  Astero
//
//  Created by Yuzhe Wang on 8/8/17.
//  Copyright © 2017 Yuzhe Wang. All rights reserved.
//

#ifndef AsteroAllocator_tpp
#define AsteroAllocator_tpp

namespace Astero {
	// categories that indicate purpose of memory being allocated
	enum MemoryCategory {
		// general purpose
		MEMCATEGORY_GENERAL = 0,
		// geometry
		MEMCATEGORY_GEOMETRY = 1,
		// animation such as bones
		MEMCATEGORY_ANIMATION = 2,
		// scene nodes
		MEMCATEGORY_SCENE_CONTROL = 3,
		// scene objects
		MEMCATEGORY_SCENE_OBJECTS = 4,
		// other resources
		MEMCATEGORY_RESOURCE = 5,
		// render system structures
		MEMCATEGORY_RENDERSYS = 6
	};
}

// memory pool policy using nedmalloc
#include "AsteroMemoryNedPooling.tpp"

namespace Astero {
	// categorized allocation policy
	template <MemoryCategory Category>
	class CategorizedAllocPolicy : public NedPoolingPolicy {
		
	};
	
	template <MemoryCategory Category, size_t Alignment = 0>
	class CategorizedAlignedAllocPolicy : public NedPoolingAlignedPolicy<Alignment> {
		
	};
	
	// Shortcuts
	typedef CategorizedAllocPolicy<MemoryCategory::MEMCATEGORY_GENERAL> GeneralAllocPolicy;
	typedef CategorizedAllocPolicy<MemoryCategory::MEMCATEGORY_GEOMETRY> GeometryAllocPolicy;
	typedef CategorizedAllocPolicy<MemoryCategory::MEMCATEGORY_ANIMATION> AnimationAllocPolicy;
	typedef CategorizedAllocPolicy<MemoryCategory::MEMCATEGORY_SCENE_CONTROL> SceneCtlAllocPolicy;
	typedef CategorizedAllocPolicy<MemoryCategory::MEMCATEGORY_SCENE_OBJECTS> SceneObjAllocPolicy;
	typedef CategorizedAllocPolicy<MemoryCategory::MEMCATEGORY_RESOURCE> ResourceAllocPolicy;
	typedef CategorizedAllocPolicy<MemoryCategory::MEMCATEGORY_RENDERSYS> RenderSysAllocPolicy;
	
	template<MemoryCategory Category>
	inline void * malloc(size_t bytes) {
		return CategorizedAllocPolicy<Category>::allocateBytes(bytes);
	}
	template<MemoryCategory Category>
	inline void * malloc_simd(size_t bytes) {
		return CategorizedAlignedAllocPolicy<Category>::allocateBytes(bytes);
	}
	template<MemoryCategory Category, size_t Alignment>
	inline void * malloc_aligned(size_t bytes) {
		return CategorizedAlignedAllocPolicy<Category, Alignment>::allocateBytes(bytes);
	}
	template<MemoryCategory Category>
	inline void free(void * ptr) {
		CategorizedAllocPolicy<Category>::deallocateBytes(ptr);
	}
	template<MemoryCategory Category>
	inline void free_simd(void * ptr) {
		CategorizedAlignedAllocPolicy<Category>::deallocateBytes(ptr);
	}
	template<MemoryCategory Category, size_t Alignment>
	inline void free_aligned(void * ptr) {
		CategorizedAlignedAllocPolicy<Category, Alignment>::deallocateBytes(ptr);
	}
//	template<typename T, MemoryCategory Category, size_t Alignment>
//	inline void new_t_aligned() {
//		new (malloc_aligned<Category, Alignment>(sizeof(T))) T;
//	}
		
	// base STL allocator class
	template<typename T>
	struct STLAllocatorBase {
		typedef T value_type;
	};
	
	// base STL allocator class. (const T version).
	template<typename T>
	struct STLAllocatorBase<const T> {
		typedef T value_type;
	};
	
	// allocator class template for STL
	template <typename T, typename AllocPolicy>
	class STLAllocator : public STLAllocatorBase<T> {
	public:
		// types
		typedef STLAllocatorBase<T>         Base;
		typedef typename Base::value_type   value_type;
		typedef T *							pointer;
		typedef const T *					const_pointer;
		typedef T &							reference;
		typedef const T &					const_reference;
		typedef std::size_t					size_type;
		typedef	std::ptrdiff_t				diference_type;
		
		// member struct
		template <typename T2>
		struct rebind {
			typedef STLAllocator<T2, AllocPolicy> other;
		};
		
		// constructor/destructor
		inline explicit STLAllocator() = default;
		inline explicit STLAllocator(const STLAllocator & alloc) = default;
		template <typename T2>
		inline STLAllocator(const STLAllocator<T2, AllocPolicy> & alloc) {
			
		}
		virtual ~STLAllocator() {
			
		}
		
		// public member functions
		inline pointer allocate(size_type n, const void * ptr = 0) {
			size_type sz = n * sizeof(T);
			return static_cast<pointer>(AllocPolicy::allocateBytes(sz));
		}
		inline void deallocate(pointer ptr, size_type n) {
			AllocPolicy::deallocateBytes(ptr);
		}
		size_type max_size() const {
			return AllocPolicy::getMaxAllocationSize();
		}
		inline pointer address(reference x) const {
			return &x;
		}
		inline const_pointer address(const_reference x) const {
			return &x;
		}
		template<typename... Args>
		void construct(pointer ptr, Args&&... args) {
			new(static_cast<void *>(ptr)) T(std::forward<Args>(args)...);
		}
		void destroy(pointer p) {
			p->~T();
		}
	};
	
	// comparing functions that allow object to be freed by allocators with same policy
	template <typename T1, typename T2, typename P>
	inline bool operator==(STLAllocator<T1, P> const & alloc1, STLAllocator<T2, P> const & alloc2) {
		return true;
	}
	template <typename T, typename P, typename OtherAllocator>
	inline bool operator==(STLAllocator<T, P> const & alloc, OtherAllocator const & other_alloc) {
		return false;
	}
	template <typename T1, typename T2, typename P>
	inline bool operator!=(STLAllocator<T1, P> const & alloc1, STLAllocator<T2, P> const & alloc2) {
		return false;
	}
	template <typename T, typename P, typename OtherAllocator>
	inline bool operator!=(STLAllocator<T, P> const & alloc, OtherAllocator const & other_alloc) {
		return true;
	}
	
} // namespace Astero

#endif // AsteroAllocator_tpp
