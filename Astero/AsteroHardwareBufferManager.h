//
//  AsteroHardwareBufferManager.h
//  Astero
//
//  Created by Yuzhe Wang on 9/21/17.
//  Copyright © 2017 Yuzhe Wang. All rights reserved.
//

#ifndef AsteroHardwareBufferManager_h
#define AsteroHardwareBufferManager_h

#include <set>
#include <mutex>
#include <map>
#include <memory> // shared_ptr

#include <GL/glew.h>
#include <OpenGL/glu.h>

#include "AsteroPrerequisites.h"
#include "AsteroHardwareBuffer.h"
#include "AsteroSingleton.tpp"

//--------------------------------------------------------------------------------------------------------------------------------
namespace Astero {
	// Abstract class representing a licensee who requests a hardware buffer copy.
	class HardwareBufferLicensee {
	public:
		virtual ~HardwareBufferLicensee() = default;
		// This method is called when buffer license is expired and is about to be returned to the shared pool.
		virtual void licenseExpired(HardwareBuffer * buf);
	};
	
	// Abstract class representing a hardware buffer manager, which is responsible for managing hardware buffers, such as
	// vertex buffer, index buffer, and temporary vertex buffer copies, and also relating classes: VertexDeclaration, and
	// VertexBufferBinding.
	class HardwareBufferManager : public Singleton<HardwareBufferManager> {
	public:
		// Buffer License Type that determines how buffer is revoked.
		enum BufferLicenseType {
			// Licensee will only release buffer when it says so.
			BLT_MANUAL_RELEASE,
			// Hardware buffer mangager can release buffer automaticly.
			BLT_AUTOMATIC_RELEASE
		};
		
		HardwareBufferManager();
		virtual ~HardwareBufferManager();
		
		// Creates a hardware vertex buffer. The meaning of contents of this vertex buffer is determined by VertexDeclaration.
		virtual HardwareVertexBufferPtr createVertexBuffer(size_t vertex_size, // Size in bytes of each vertex
																 size_t vertex_num, // Number of vertices in buffer
																 HardwareBuffer::Usage usage, // Buffer usage enumeration.
																 bool use_shadow_buffer = false // Whether to use shadow buffer in system memory.
																 ) = 0;
		// Creates a hardware index buffer.
		virtual HardwareIndexBufferPtr createIndexBuffer(HardwareIndexBuffer::IndexType itype, // Type in index, 16- or 32-bit.
															   size_t index_num, // Number of indices in buffer.
															   HardwareBuffer::Usage usage, // Buffer usage enumeration.
															   bool use_shadow_buffer = false // Whether to use shadow buffer in system memory.
															   ) = 0;
		// Creates a render to vertex buffer.
		virtual RenderToVertexBufferPtr createRenderToVertexBuffer() = 0;
		// Creates a new vertex declaration.
		virtual VertexDeclaration * createVertexDeclaration();
		// Creates a new vertex buffer binding.
		virtual VertexBufferBinding * createVertexBufferBinding();
		// Destroys a vertex declaration.
		virtual void destroyVertexDeclaration(VertexDeclaration * decl);
		// Destroys a vertex buffer binding.
		virtual void destroyVertexBufferBinding(VertexBufferBinding * binding);
		// Registers an existing buffer as a temporary copy of another.
		virtual void registerVertexBufferSourceAndCopy(const HardwareVertexBufferPtr & source_buffer,
													   const HardwareVertexBufferPtr & copy);
		// Allocates a copy of a given vertex buffer.
		virtual HardwareVertexBufferPtr allocateVertexBufferCopy(const HardwareVertexBufferPtr & source_buffer, // source buffer for copy.
																	   BufferLicenseType license_type, // enumeration of license type.
																	   HardwareBufferLicensee* licensee, // pointer to class who requests the copy.
																 bool copy_data = false);
		// Manually releases a vertex buffer copy for others to subsequently use.
		virtual void releaseVertexBufferCopy(const HardwareVertexBufferPtr & copy);
		// Tells hareware buffer manager that a copy is intent to reuse.
		virtual void touchVertexBufferCopy(const HardwareVertexBufferPtr & copy);
		// Frees all unused vertex buffer copies.
		virtual void freeUnusedBufferCopies();
		// Releases all expired temporary buffer which have been allocated using BLT_AUTOMATIC_RELEASE.
		// If force_free_unused is true, also frees unused temporary buffers.
		// If force_free_unused is false, frees unused temporary buffers when frames of utilization below
		// half reaches UNDER_USED_FRAME_THRESHOLD.
		virtual void releaseBufferCopies(bool force_free_unused = false);
		// Forces the release of a given buffer copy.
		virtual void forceReleaseBufferCopies(const HardwareVertexBufferPtr & source);
		// Forces the release of a given buffer copy.
		virtual void forceReleaseBufferCopies(HardwareVertexBuffer * source);
		// Notifies that a hardware vertex buffer has been destroyed.
		void notifyVertexBufferDestroyed(HardwareVertexBuffer * buffer);
		
	protected:
		typedef std::set<HardwareVertexBuffer *> VertexBufferList;
		typedef std::set<HardwareIndexBuffer *> IndexBufferList;
		typedef std::set<VertexDeclaration *> VertexDeclarationList;
		typedef std::set<VertexBufferBinding *> VertexBufferBindingList;
		// Struct that holds info of a license to use a temporary shared buffer.
		struct VertexBufferLicense {
			VertexBufferLicense(HardwareVertexBuffer * origin,
								BufferLicenseType ltype,
								size_t delay,
								HardwareVertexBufferPtr buf,
								HardwareBufferLicensee * lic)
									: original_buffer(origin),license_type(ltype), expired_delay(delay), buffer(buf), licensee(lic) {
				
			}
			// original hardware vertex buffer pointer.
			HardwareVertexBuffer * original_buffer;
			// license type of temporary buffer.
			BufferLicenseType license_type;
			// number of frames to wait to release after license expired.
			size_t expired_delay;
			// temporary hardware vertex buffer.
			HardwareVertexBufferPtr buffer;
			// licensee of temporary buffer.
			HardwareBufferLicensee * licensee;
		};
		// Map from original buffer to temporary buffer.
		typedef std::multimap<HardwareVertexBuffer *, HardwareVertexBufferPtr>
		FreeTemporaryVertexBufferMap;
		// Map from temporary buffer to detail of license.
		typedef std::map<HardwareVertexBuffer *, VertexBufferLicense> TemporaryVertexBufferLicenseMap;
		typedef std::recursive_mutex Mutex;
		typedef std::lock_guard<Mutex> Lock;
		
		// Internal method for creating a new VertexDeclaration.
		virtual VertexDeclaration * createVertexDeclarationImpl();
		// Internal method for creating a new VertexBufferBinding.
		virtual VertexBufferBinding * createVertexBufferBindingImpl();
		// Internal method for destroying a VertexDeclaration.
		virtual void destroyVertexDeclarationImpl(VertexDeclaration * decl);
		// Internal method for destroying a VertexBufferBinding.
		virtual void destroyVertexBufferBindingImpl(VertexBufferBinding * binding);
		// Internal method for destroying all VertexDeclarations.
		virtual void destroyAllVertexDeclarations();
		// Internal method for destroying all VertexBufferBindings.
		virtual void destroyAllVertexBufferBindings();
		// Creates a new buffer as a copy of the source, without actually copying the data.
		virtual HardwareVertexBufferPtr makeBufferCopy(const HardwareVertexBufferPtr & source,
															 HardwareBuffer::Usage usage,
															 bool use_shadow_buffer);
		
		// Data members
		// Number of frames to wait before free unused temporary buffer.
		static const size_t UNDER_USED_FRAME_THRESHOLD;
		// Number of frames to wait for BLT_AUTOMATIC_RELEASE temporary buffer.
		static const size_t EXPIRED_DELAY_FRAME_THRESHOLD;
		// Number of frames elapsed since temporary buffer is under used.
		size_t under_used_frame_count_;
		// Lists of hardware buffers.
		VertexBufferList vertex_buffer_list_;
		IndexBufferList index_buffer_list_;
		VertexDeclarationList vertex_declaration_list_;
		VertexBufferBindingList vertex_buffer_binding_list_;
		FreeTemporaryVertexBufferMap free_temporary_vertex_buffer_map_;
		TemporaryVertexBufferLicenseMap temporary_vertex_buffer_license_map_;
		// Mutexes
		Mutex vertex_buffer_mutex_;
		Mutex index_buffer_mutex_;
		Mutex vertex_declaration_mutex_;
		Mutex vertex_buffer_binding_mutex_;
		Mutex temporary_buffer_mutex_;
		
	};
} // namespace Astero
//--------------------------------------------------------------------------------------------------------------------------------

#include "AsteroAllocator.tpp"
#include "AsteroGLStateCacheManager.h"

#define GL_DEFAULT_MAP_BUFFER_THRESHOLD (1024*32)
#define SCRATCH_ALIGNMENT 32
#define SCRATCH_POOL_SIZE 1 * 1024 * 1024

namespace Astero {

	struct GLScratchBufferAlloc {
		unsigned int size: 31;
		unsigned int free: 1;
	};

	// HardwareBufferManager for OpenGL
	class GLHardwareBufferManager : public HardwareBufferManager {
	public:
		GLHardwareBufferManager();
		~GLHardwareBufferManager();
		
		// Getters and setters.
		size_t getGLMapBufferThreshold() const;
		void setGLMapBufferThreshold(const size_t value);
		GLStateCacheManager * getStateCacheManager();
		// Creates a vertex buffer.
		HardwareVertexBufferPtr createVertexBuffer(size_t vertex_size, // Size in bytes of each vertex
														 size_t vertex_num, // Number of vertices in buffer
														 HardwareBuffer::Usage usage, // Buffer usage enumeration.
														 bool use_shadow_buffer = false) override;
		HardwareIndexBufferPtr createIndexBuffer(HardwareIndexBuffer::IndexType index_type,
													   size_t index_num,
													   HardwareBuffer::Usage usage,
												 bool use_shadow_buffer = false) override;
		RenderToVertexBufferPtr createRenderToVertexBuffer() override;
		// Utility function to get corresponding GLenum usage given HBU usage.
		static GLenum getGLUsage(unsigned int usage);
		// Utility function to get corresponding GLenum type given VET type.
		static GLenum getGLType(unsigned int type);
		void * allocateScratch(unsigned int size);
		void deallocateScratch(void * ptr);
		
	protected:
		typedef std::recursive_mutex Mutex;
		typedef std::lock_guard<Mutex> Lock;
		
		GLStateCacheManager * state_cache_manager_;
		char * scratch_buffer_pool_;
		size_t map_buffer_threshold_;
		Mutex scratch_mutex_;
		
	};
}


#endif // AsteroHardwareBufferManager_h
