//
//  AsteroHardwareBufferManager.cpp
//  Astero
//
//  Created by Yuzhe Wang on 10/3/17.
//  Copyright © 2017 Yuzhe Wang. All rights reserved.
//

#include <stdio.h>

#include "AsteroHardwareBufferManager.h"

namespace Astero {
	//--------------------------------------------------------------------------------------------------------------------------------
	HardwareBufferManager::HardwareBufferManager() : under_used_frame_count_(0) {
		
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	HardwareBufferManager::~HardwareBufferManager(){
		vertex_buffer_list_.clear();
		index_buffer_list_.clear();
		destroyAllVertexDeclarations();
		destroyAllVertexBufferBindings();
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	VertexDeclaration * HardwareBufferManager::createVertexDeclaration() {
		VertexDeclaration * decl = createVertexDeclarationImpl();
		Lock lock(vertex_buffer_mutex_);
		vertex_declaration_list_.insert(decl);
		return decl;
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	VertexBufferBinding * HardwareBufferManager::createVertexBufferBinding() {
		VertexBufferBinding * binding = createVertexBufferBindingImpl();
		Lock lock(vertex_buffer_binding_mutex_);
		vertex_buffer_binding_list_.insert(binding);
		return binding;
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	void HardwareBufferManager::destroyVertexDeclaration(VertexDeclaration * decl) {
		Lock lock(vertex_buffer_mutex_);
		vertex_declaration_list_.erase(decl);
		destroyVertexDeclarationImpl(decl);
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	void HardwareBufferManager::destroyVertexBufferBinding(VertexBufferBinding * binding) {
		Lock lock(vertex_buffer_binding_mutex_);
		vertex_buffer_binding_list_.erase(binding);
		destroyVertexBufferBindingImpl(binding);
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	void HardwareBufferManager::registerVertexBufferSourceAndCopy(const HardwareVertexBufferPtr & source_buffer,
												   const HardwareVertexBufferPtr & copy) {
		Lock lock(temporary_buffer_mutex_);
		free_temporary_vertex_buffer_map_.insert(FreeTemporaryVertexBufferMap::value_type(source_buffer.get(), copy));
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	HardwareVertexBufferPtr HardwareBufferManager::allocateVertexBufferCopy(const HardwareVertexBufferPtr & source_buffer, // source buffer for copy.
															 BufferLicenseType license_type, // enumeration of license type.
															 HardwareBufferLicensee* licensee, // pointer to class who requests the copy.
															 bool copy_data) { // Whether to copy data and the structure.
		Lock vertex_buffer_lock(vertex_buffer_mutex_);
		Lock temp_buffer_lock(temporary_buffer_mutex_);
		HardwareVertexBufferPtr vbuf;
		auto iter = free_temporary_vertex_buffer_map_.find(source_buffer.get());
		// If source_buffer's copy does not exist in free temporary vertex buffer pool, creates one.
		if (iter == free_temporary_vertex_buffer_map_.end()) {
			vbuf = makeBufferCopy(source_buffer, HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE, true);
		}
		// If souce_buffer's copy exists, returns the copy's address.
		else {
			vbuf = iter->second;
			free_temporary_vertex_buffer_map_.erase(iter);
		}
		// Copies data from source buffer to copy.
		if (copy_data) {
			vbuf->copyData(*(source_buffer.get()), 0, 0, source_buffer->getSizeInBytes(), true);
		}
		// Assigns temporary buffer with a license.
		temporary_vertex_buffer_license_map_.insert(
													TemporaryVertexBufferLicenseMap::value_type(
																								vbuf.get(),
																								VertexBufferLicense(source_buffer.get(), license_type, EXPIRED_DELAY_FRAME_THRESHOLD, vbuf, licensee)));
		return vbuf;
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	void HardwareBufferManager::releaseVertexBufferCopy(const HardwareVertexBufferPtr & copy) {
		Lock lock(temporary_buffer_mutex_);
		auto iter = temporary_vertex_buffer_license_map_.find(copy.get());
		if (iter != temporary_vertex_buffer_license_map_.end()) {
			const VertexBufferLicense & vbl = iter->second;
			// Informs licensee that license is expired.
			vbl.licensee->licenseExpired(vbl.buffer.get());
			free_temporary_vertex_buffer_map_.insert(
													 FreeTemporaryVertexBufferMap::value_type(vbl.original_buffer, vbl.buffer));
			temporary_vertex_buffer_license_map_.erase(iter);
		}
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	void HardwareBufferManager::touchVertexBufferCopy(const HardwareVertexBufferPtr & copy) {
		Lock lock(temporary_buffer_mutex_);
		auto iter = temporary_vertex_buffer_license_map_.find(copy.get());
		if (iter != temporary_vertex_buffer_license_map_.end()) {
			VertexBufferLicense & vbl = iter->second;
			assert(vbl.license_type == BLT_AUTOMATIC_RELEASE);
			vbl.expired_delay = EXPIRED_DELAY_FRAME_THRESHOLD;
		}
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	void HardwareBufferManager::freeUnusedBufferCopies() {
		Lock lock(temporary_buffer_mutex_);
		auto iter = free_temporary_vertex_buffer_map_.begin();
		while (iter != free_temporary_vertex_buffer_map_.end()) {
			auto cur_iter = iter++;
			if (cur_iter->second.use_count() <= 1) {
				free_temporary_vertex_buffer_map_.erase(cur_iter);
			}
		}
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	void HardwareBufferManager::releaseBufferCopies(bool force_free_unused) {
		Lock lock(temporary_buffer_mutex_);
		size_t num_unused = free_temporary_vertex_buffer_map_.size();
		size_t num_used = temporary_vertex_buffer_license_map_.size();
		// Erases copies which are automatic licensed out.
		auto iter = temporary_vertex_buffer_license_map_.begin();
		while (iter != temporary_vertex_buffer_license_map_.end()) {
			auto cur_iter = iter++;
			VertexBufferLicense & vbl = cur_iter->second;
			if (vbl.license_type == BLT_AUTOMATIC_RELEASE && --vbl.expired_delay <= 0) {
				vbl.licensee->licenseExpired(vbl.buffer.get());
				free_temporary_vertex_buffer_map_.insert(
														 FreeTemporaryVertexBufferMap::value_type(vbl.original_buffer, vbl.buffer));
				temporary_vertex_buffer_license_map_.erase(cur_iter);
			}
		}
		if (force_free_unused) {
			freeUnusedBufferCopies();
			under_used_frame_count_ = 0;
		}
		// Automatically frees unused buffer copies.
		else {
			if (num_used < num_unused) {
				if (++under_used_frame_count_ >= UNDER_USED_FRAME_THRESHOLD) {
					freeUnusedBufferCopies();
					under_used_frame_count_ = 0;
				}
			}
			else {
				under_used_frame_count_ = 0;
			}
		}
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	void HardwareBufferManager::forceReleaseBufferCopies(const HardwareVertexBufferPtr & source) {
		forceReleaseBufferCopies(source.get());
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	void HardwareBufferManager::forceReleaseBufferCopies(HardwareVertexBuffer * source) {
		Lock lock(temporary_buffer_mutex_);
		auto iter = temporary_vertex_buffer_license_map_.begin();
		while (iter != temporary_vertex_buffer_license_map_.end()) {
			auto cur_iter = iter++;
			const VertexBufferLicense & vbl = cur_iter->second;
			if (vbl.original_buffer == source) {
				vbl.licensee->licenseExpired(vbl.buffer.get());
				temporary_vertex_buffer_license_map_.erase(cur_iter);
			}
		}
		free_temporary_vertex_buffer_map_.erase(source);
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	void HardwareBufferManager::notifyVertexBufferDestroyed(HardwareVertexBuffer * buffer) {
		Lock lock(vertex_buffer_mutex_);
		auto iter = vertex_buffer_list_.find(buffer);
		if (iter != vertex_buffer_list_.end()) {
			vertex_buffer_list_.erase(iter);
			forceReleaseBufferCopies(buffer);
		}
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	VertexDeclaration * HardwareBufferManager::createVertexDeclarationImpl() {
		return new VertexDeclaration();
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	VertexBufferBinding * HardwareBufferManager::createVertexBufferBindingImpl() {
		return new VertexBufferBinding();
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	void HardwareBufferManager::destroyVertexDeclarationImpl(VertexDeclaration * decl) {
		delete decl;
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	void HardwareBufferManager::destroyVertexBufferBindingImpl(VertexBufferBinding * binding) {
		delete binding;
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	void HardwareBufferManager::destroyAllVertexDeclarations() {
		Lock lock(vertex_declaration_mutex_);
		for (auto iter = vertex_declaration_list_.begin(); iter != vertex_declaration_list_.end(); ++iter) {
			destroyVertexDeclarationImpl(*iter);
		}
		vertex_declaration_list_.clear();
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	void HardwareBufferManager::destroyAllVertexBufferBindings() {
		Lock lock(vertex_buffer_binding_mutex_);
		for (auto iter = vertex_buffer_binding_list_.begin(); iter != vertex_buffer_binding_list_.end(); ++iter) {
			destroyVertexBufferBindingImpl(*iter);
		}
		vertex_buffer_binding_list_.clear();
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	HardwareVertexBufferPtr HardwareBufferManager::makeBufferCopy(const HardwareVertexBufferPtr & source,
												   HardwareBuffer::Usage usage,
												   bool use_shadow_buffer) {
		return createVertexBuffer(source->getVertexSize(), source->getVertexNum(), usage, use_shadow_buffer);
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	GLHardwareBufferManager::GLHardwareBufferManager()
	: scratch_buffer_pool_(nullptr), map_buffer_threshold_(GL_DEFAULT_MAP_BUFFER_THRESHOLD) {
		state_cache_manager_ = nullptr;
		scratch_buffer_pool_ = static_cast<char *>(malloc_aligned<MEMCATEGORY_GEOMETRY, SCRATCH_ALIGNMENT>(SCRATCH_POOL_SIZE));
		GLScratchBufferAlloc * head_alloc = reinterpret_cast<GLScratchBufferAlloc*>(scratch_buffer_pool_);
		head_alloc->size = SCRATCH_POOL_SIZE - sizeof(GLScratchBufferAlloc);
		head_alloc->free = 1;
		map_buffer_threshold_ = 0;
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	GLHardwareBufferManager::~GLHardwareBufferManager() {
		destroyAllVertexDeclarations();
		destroyAllVertexBufferBindings();
		free_aligned<MEMCATEGORY_GEOMETRY, SCRATCH_ALIGNMENT>(scratch_buffer_pool_);
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	size_t GLHardwareBufferManager::getGLMapBufferThreshold() const {
		return map_buffer_threshold_;
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	void GLHardwareBufferManager::setGLMapBufferThreshold(const size_t value) {
		map_buffer_threshold_ = value;
	}
	GLStateCacheManager * GLHardwareBufferManager::getStateCacheManager() {
		return state_cache_manager_;
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	HardwareVertexBufferPtr GLHardwareBufferManager::createVertexBuffer(size_t vertex_size, // Size in bytes of each vertex
											   size_t vertex_num, // Number of vertices in buffer
											   HardwareBuffer::Usage usage, // Buffer usage enumeration.
											   bool use_shadow_buffer) {
		GLHardwareVertexBuffer * buf = new GLHardwareVertexBuffer(this, vertex_size, vertex_num, usage, use_shadow_buffer);
		Lock lock(vertex_buffer_mutex_);
		vertex_buffer_list_.insert(buf);
		return HardwareVertexBufferPtr(buf);
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	HardwareIndexBufferPtr GLHardwareBufferManager::createIndexBuffer(HardwareIndexBuffer::IndexType index_type,
											 size_t index_num,
											 HardwareBuffer::Usage usage,
											 bool use_shadow_buffer) {
		GLHardwareIndexBuffer * buf = new GLHardwareIndexBuffer(this, index_type, index_num, usage, use_shadow_buffer);
		Lock lock(index_buffer_mutex_);
		index_buffer_list_.insert(buf);
		return HardwareIndexBufferPtr(buf);
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	RenderToVertexBufferPtr GLHardwareBufferManager::createRenderToVertexBuffer() {
		return RenderToVertexBufferPtr(new GLRenderToVertexBuffer);
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	GLenum GLHardwareBufferManager::getGLUsage(unsigned int usage) {
		switch (usage) {
			case HardwareBuffer::HBU_STATIC:
			case HardwareBuffer::HBU_STATIC_WRITE_ONLY:
				return GL_STATIC_DRAW_ARB;
			case HardwareBuffer::HBU_DYNAMIC:
			case HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY:
				return GL_DYNAMIC_DRAW_ARB;
			case HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE:
				return GL_STREAM_DRAW_ARB;
			default:
				return GL_DYNAMIC_DRAW_ARB;
		}
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	GLenum GLHardwareBufferManager::getGLType(unsigned int type) {
		switch (type) {
			case VET_FLOAT1:
			case VET_FLOAT2:
			case VET_FLOAT3:
			case VET_FLOAT4:
				return GL_FLOAT;
			case VET_SHORT1:
			case VET_SHORT2:
			case VET_SHORT3:
			case VET_SHORT4:
				return GL_SHORT;
			case VET_COLOUR:
			case VET_COLOUR_ABGR:
			case VET_COLOUR_ARGB:
			case VET_UBYTE4:
				return GL_UNSIGNED_BYTE;
			default:
				return 0;
		}
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	void * GLHardwareBufferManager::allocateScratch(unsigned int size) {
		Lock lock(scratch_mutex_);
		if (size % 4 != 0)
			size += 4 - (size % 4);
		unsigned int pos = 0;
		GLScratchBufferAlloc * cur = nullptr;
		while (pos < SCRATCH_POOL_SIZE) {
			cur = (GLScratchBufferAlloc *)(scratch_buffer_pool_ + pos);
			if (cur->free && cur->size >= size) {
				if (cur->size > size + sizeof(GLScratchBufferAlloc)) {
					unsigned int offset = (unsigned int)sizeof(GLScratchBufferAlloc) + size;
					GLScratchBufferAlloc * next = cur + offset;
					next->free = 1;
					next->size = cur->size - offset;
					cur->size = size;
				}
				cur->free = 0;
				return ++cur;
			}
			pos += (unsigned int)sizeof(GLScratchBufferAlloc) + cur->size;
		}
		return nullptr;
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	void GLHardwareBufferManager::deallocateScratch(void * ptr) {
		Lock lock(scratch_mutex_);
		unsigned int pos = 0;
		GLScratchBufferAlloc * prev = nullptr;
		GLScratchBufferAlloc * cur = nullptr;
		GLScratchBufferAlloc * next = nullptr;
		while (pos < SCRATCH_POOL_SIZE) {
			cur = reinterpret_cast<GLScratchBufferAlloc*>(scratch_buffer_pool_) + pos;
			if (cur + sizeof(GLScratchBufferAlloc) == ptr) {
				cur->free = 1;
				if (prev && prev->free) {
					pos -= prev->size + sizeof(GLScratchBufferAlloc);
					prev->size += cur->size + sizeof(GLScratchBufferAlloc);
					cur = prev;
				}
				if (pos + cur->size + sizeof(GLScratchBufferAlloc) < SCRATCH_POOL_SIZE) {
					next = cur + cur->size + sizeof(GLScratchBufferAlloc);
					if (next->free) {
						cur->size += next->size + sizeof(GLScratchBufferAlloc);
					}
				}
				return;
			}
			pos += sizeof(GLScratchBufferAlloc) + cur->size;
			prev = cur;
		}
		assert(false);
	}
} // namespace Astero
