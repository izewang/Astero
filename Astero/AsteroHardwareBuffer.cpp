//
//  AsteroHardwareBuffer.cpp
//  Astero
//
//  Created by Yuzhe Wang on 10/4/17.
//  Copyright © 2017 Yuzhe Wang. All rights reserved.
//

#include <stdio.h>

#include "AsteroHardwareBuffer.h"
#include "AsteroHardwareBufferManager.h"

namespace Astero {
	HardwareBuffer::HardwareBuffer(Usage usage, bool use_system_memory, bool use_shadow_buffer)
	: size_in_bytes_(0), usage_(usage), locked_(false), lock_offset_(false), lock_size_(false), use_system_memory_(use_system_memory), use_shadow_buffer_(use_shadow_buffer), shadow_buffer_(nullptr), shadow_updated_(false), suppress_hardware_update_(false) {
		
	}
	HardwareBuffer::~HardwareBuffer() {}
	void * HardwareBuffer::lock(size_t offset, size_t size, LockOption option) {
		return nullptr;
	}
	void * HardwareBuffer::lock(LockOption) {
		return nullptr;
	}
	void HardwareBuffer::unlock(void) {
		
	}
	void HardwareBuffer::copyData(HardwareBuffer & src_buffer, size_t src_offset,
				  size_t dest_offset, size_t size,
								  bool discard_whole_buffer) {
		
	}
	void HardwareBuffer::copyData(HardwareBuffer & src_buffer) {
		
	}
	void HardwareBuffer::updateFromShadow() {
		
	}
	size_t HardwareBuffer::getSizeInBytes() const {
		return size_in_bytes_;
	}
	HardwareBuffer::Usage HardwareBuffer::getUsage() const {
		return usage_;
	}
	bool HardwareBuffer::hasShadowBuffer() const {
		return use_shadow_buffer_;
	}
	bool HardwareBuffer::isLocked() const {
		return locked_ || (use_shadow_buffer_ && shadow_buffer_->isLocked());
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	// HardwareVertexBuffer
	//--------------------------------------------------------------------------------------------------------------------------------
	HardwareVertexBuffer::HardwareVertexBuffer(HardwareBufferManager * manager,
						 size_t vertex_size,
						 size_t vertex_num,
						 HardwareBuffer::Usage usage,
						 bool use_system_memory, bool use_shadow_buffer)
	: HardwareBuffer(usage, use_system_memory, use_shadow_buffer), manager_(manager),
	vertex_size_(vertex_size), vertex_num_(vertex_num), is_instance_data_(false), instance_data_step_rate_(1){
		size_in_bytes_ = vertex_size_ * vertex_num_;
		if (use_shadow_buffer_) {
			shadow_buffer_ = new DefaultHardwareVertexBuffer(vertex_size_, vertex_num_, HardwareBuffer::HBU_DYNAMIC);
		}
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	HardwareVertexBuffer::~HardwareVertexBuffer() {
		delete shadow_buffer_;
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	HardwareBufferManager * HardwareVertexBuffer::getManager() const {
		return manager_;
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	size_t HardwareVertexBuffer::getVertexSize() const {
		return vertex_size_;
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	size_t HardwareVertexBuffer::getVertexNum() const {
		return vertex_num_;
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	bool HardwareVertexBuffer::getIsInstanceData() const {
		return is_instance_data_;
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	void HardwareVertexBuffer::setIsInstanceData(const bool val) {
		is_instance_data_ = val;
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	unsigned int HardwareVertexBuffer::getInstanceDataStepRate() const {
		return instance_data_step_rate_;
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	void HardwareVertexBuffer::setInstanceDataStepRate(const unsigned int val) {
		if (val > 0)
			instance_data_step_rate_ = val;
		else {
			abort();
		}
	}
} // namespace Astero

#include "AsteroAllocator.tpp"
namespace Astero {
	//--------------------------------------------------------------------------------------------------------------------------------
	// DefaultHardwareVertexBuffer
	//--------------------------------------------------------------------------------------------------------------------------------
	DefaultHardwareVertexBuffer::DefaultHardwareVertexBuffer(size_t vertex_size,
															 size_t vertex_num,
															 HardwareBuffer::Usage usage)
	: HardwareVertexBuffer(nullptr, vertex_size, vertex_num, usage, true, false), data_(nullptr) {
		data_ = static_cast<unsigned char *>(malloc_simd<MEMCATEGORY_GEOMETRY>(size_in_bytes_));
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	DefaultHardwareVertexBuffer::~DefaultHardwareVertexBuffer() {
		free_simd<MEMCATEGORY_GEOMETRY>(data_);
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	void * DefaultHardwareVertexBuffer::lock(size_t offset, size_t size, LockOption option) {
		locked_ = true;
		return data_ + offset;
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	void  DefaultHardwareVertexBuffer::unlock() {
		locked_ = false;
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	void DefaultHardwareVertexBuffer::readData(size_t offset, size_t size, void * dest) {
		assert((offset + size) <= size_in_bytes_);
		memcpy(dest, data_ + offset, size);
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	void DefaultHardwareVertexBuffer::writeData(size_t offset, size_t size, const void * src, bool discard_whole_buffer) {
		assert((offset + size) <= size_in_bytes_);
		memcpy(data_ + offset, src, size);
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	void * DefaultHardwareVertexBuffer::lockImpl(size_t offset, size_t size, LockOption option) {
		return data_ + offset;
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	void DefaultHardwareVertexBuffer::unlockImpl() {
		
	}
} // namespace Astero

namespace Astero {
	//--------------------------------------------------------------------------------------------------------------------------------
	// GLHardwareVertexBuffer
	//--------------------------------------------------------------------------------------------------------------------------------
	GLHardwareVertexBuffer::GLHardwareVertexBuffer(HardwareBufferManager * manager,
												   size_t vertex_size,
												   size_t vertex_num,
												   HardwareBuffer::Usage usage,
												   bool use_shadow_buffer)
	: HardwareVertexBuffer(manager, vertex_size, vertex_num, usage, false, use_shadow_buffer), locked_to_scratch_(false), scratch_upload_on_unlock_(false), scratch_offset_(0), scratch_size_(0), scratch_(nullptr) {
		glGenBuffersARB(1, &buffer_id_);
		assert(buffer_id_);
		static_cast<GLHardwareBufferManager *>(manager)->getStateCacheManager()->bindGLBuffer(GL_ARRAY_BUFFER_ARB, buffer_id_);
		// Initializes buffer and set usage.
		glBufferDataARB(GL_ARRAY_BUFFER_ARB, size_in_bytes_, nullptr, GLHardwareBufferManager::getGLUsage(usage));
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	GLHardwareVertexBuffer::~GLHardwareVertexBuffer() {
		static_cast<GLHardwareBufferManager *>(manager_)->getStateCacheManager()->deleteGLBuffer(GL_ARRAY_BUFFER_ARB, buffer_id_);
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	GLuint GLHardwareVertexBuffer::getGLBufferId() const {
		return buffer_id_;
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	void GLHardwareVertexBuffer::readData(size_t offset, size_t size, void * dest) {
		if (use_shadow_buffer_) {
			// Reads data from shadow buffer.
			void * src = shadow_buffer_->lock(offset, size, HBL_READ_ONLY);
			memcpy(dest, src, size);
			shadow_buffer_->unlock();
		}
		else {
			// Reads data from real buffer.
			static_cast<GLHardwareBufferManager *>(manager_)->getStateCacheManager()->bindGLBuffer(GL_ARRAY_BUFFER_ARB, buffer_id_);
			glGetBufferSubDataARB(GL_ARRAY_BUFFER_ARB, offset, size, dest);
		}
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	void GLHardwareVertexBuffer::writeData(size_t offset, size_t size, const void * src,
										   bool discard_whole_buffer) {
		// Updates shadow buffer.
		if (use_shadow_buffer_) {
			void * dest = shadow_buffer_->lock(offset, size, discard_whole_buffer ? HBL_DISCARD : HBL_WRITE_ONLY);
			memcpy(dest, src, size);
			shadow_buffer_->unlock();
		}
		// Updates vertex buffer.
		static_cast<GLHardwareBufferManager *>(manager_)->getStateCacheManager()->bindGLBuffer(GL_ARRAY_BUFFER_ARB, buffer_id_);
		if (offset == 0 && size == size_in_bytes_) {
			glBufferDataARB(GL_ARRAY_BUFFER_ARB, size, src, GLHardwareBufferManager::getGLUsage(usage_));
		}
		else {
			if (discard_whole_buffer) {
				glBufferDataARB(GL_ARRAY_BUFFER_ARB, size_in_bytes_, nullptr, GLHardwareBufferManager::getGLUsage(usage_));
			}
			glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, offset, size, src);
		}
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	void GLHardwareVertexBuffer::updateFromShadow() {
		if (use_shadow_buffer_ && shadow_updated_ && !suppress_hardware_update_) {
			const void * src = shadow_buffer_->lock(lock_offset_, lock_size_, HBL_READ_ONLY);
			static_cast<GLHardwareBufferManager *>(manager_)->getStateCacheManager()->bindGLBuffer(GL_ARRAY_BUFFER_ARB, buffer_id_);
			if (lock_offset_ == 0 && lock_size_ == size_in_bytes_) {
				glBufferDataARB(GL_ARRAY_BUFFER_ARB, size_in_bytes_, src, GLHardwareBufferManager::getGLUsage(usage_));
			}
			else {
				glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, lock_offset_, lock_size_, src);
			}
			shadow_buffer_->unlock();
			shadow_updated_ = false;
		}
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	void * GLHardwareVertexBuffer::lockImpl(size_t offset, size_t size, LockOption option) {
		if (locked_) {
			return nullptr;
		}
		void * ret = nullptr;
		GLHardwareBufferManager * gl_buffer_manager = static_cast<GLHardwareBufferManager*>(HardwareBufferManager::getSingletonPtr());
		// If buffer size is smaller enough, uses scratch buffer instead.
		if (size < gl_buffer_manager->getGLMapBufferThreshold()) {
			ret = gl_buffer_manager->allocateScratch((unsigned int)size);
			if (ret) {
				locked_to_scratch_ = true;
				scratch_ = ret;
				// If LockOption is not discard nor no_overwrite, reads data from real buffer to scratch buffer.
				if (option != HBL_DISCARD && option != HBL_NO_OVERWRITE)
					readData(offset, size, ret);
			}
		}
		// Scratches allocation failed or size is above threshold.
		if(!ret) {
			locked_to_scratch_ = false;
			// Uses glMapBuffer.
			static_cast<GLHardwareBufferManager *>(manager_)->getStateCacheManager()->bindGLBuffer(GL_ARRAY_BUFFER_ARB, buffer_id_);
			if (option == HBL_DISCARD || option == HBL_NO_OVERWRITE) {
				// Discards the buffer.
				glBufferDataARB(GL_ARRAY_BUFFER_ARB, size_in_bytes_, nullptr, GLHardwareBufferManager::getGLUsage(usage_));
				GLenum error = glGetError();
				if (error) {
					static_cast<GLHardwareBufferManager *>(manager_)->getStateCacheManager()->deleteGLBuffer(GL_ARRAY_BUFFER_ARB, buffer_id_);
					buffer_id_ = 0;
					glGenBuffersARB(1, &buffer_id_);
					static_cast<GLHardwareBufferManager *>(manager_)->getStateCacheManager()->bindGLBuffer(GL_ARRAY_BUFFER_ARB, buffer_id_);
					glBufferDataARB(GL_ARRAY_BUFFER_ARB, size_in_bytes_, nullptr, GLHardwareBufferManager::getGLUsage(usage_));
				}
			}
			GLenum access = 0;
			if (usage_ & HBU_WRITE_ONLY)
				access = GL_WRITE_ONLY_ARB;
			else if (option == HBL_READ_ONLY)
				access = GL_READ_ONLY_ARB;
			else
				access = GL_READ_WRITE_ARB;
			void * buffer = glMapBufferARB(GL_ARRAY_BUFFER_ARB, access);
			assert(buffer);
			// Returns offsetted pointer.
			ret = static_cast<void *>(static_cast<unsigned char *>(buffer) + offset);
		}
		locked_ = true;
		return ret;
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	void GLHardwareVertexBuffer::unlockImpl() {
		if (locked_to_scratch_) {
			if (scratch_upload_on_unlock_) {
				// Writes data back to vertex buffer from scratch buffer.
				writeData(scratch_offset_, scratch_size_, scratch_, scratch_offset_ == 0 && scratch_size_ == getSizeInBytes());
			}
			// Deallocates memory from scratch buffer.
			static_cast<GLHardwareBufferManager*>(HardwareBufferManager::getSingletonPtr())->deallocateScratch(scratch_);
			locked_to_scratch_ = false;
		}
		else {
			// Uses glUnmapBuffer.
			static_cast<GLHardwareBufferManager *>(manager_)->getStateCacheManager()->bindGLBuffer(GL_ARRAY_BUFFER_ARB, buffer_id_);
			assert(glUnmapBufferARB(GL_ARRAY_BUFFER_ARB));
		}
		locked_ = false;
	}
}
