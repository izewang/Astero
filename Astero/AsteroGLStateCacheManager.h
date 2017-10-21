//
//  AsteroGLStateCacheManager.h
//  Astero
//
//  Created by Yuzhe Wang on 10/9/17.
//  Copyright © 2017 Yuzhe Wang. All rights reserved.
//

#ifndef AsteroGLStateCacheManager_h
#define AsteroGLStateCacheManager_h

#include "AsteroPrerequisites.h"

namespace Astero {
	class GLStateCacheManagerImp;
	// This class stores OpenGL state in memory to save unnecessary state change performed by OpenGL.
	class GLStateCacheManager {
	public:
		GLStateCacheManager() = default;
		~GLStateCacheManager() = default;
		
		// Drops all recorded state for a given context.
		void unregisterContext(intptr_t id);
		// Switch to context.
		void switchContext(intptr_t id);
		// Clears all stored states in current active cache.
		void clearCache();
		// Binds an OpenGL buffer to target.
		void bindGLBuffer(GLenum target, GLuint buffer, bool force = false);
		// Deletes an OpenGL buffer.
		void deleteGLBuffer(GLenum target, GLuint buffer, bool force = false);
		
	protected:
		typedef std::unordered_map<intptr_t, GLStateCacheManagerImp *> CacheMap;
		CacheMap cache_map_;
		GLStateCacheManagerImp * imp_;
	};
	
	//--------------------------------------------------------------------------------------------------------------------------------
	class GLStateCacheManagerImp {
	public:
		void initializeCache();
		void clearCache();
		void bindGLBuffer(GLenum target, GLuint buffer, bool force = false);
		void deleteGLBuffer(GLenum target, GLuint buffer, bool force = false);
		
	private:
		typedef std::unordered_map<GLenum, GLuint> BindBufferMap;
		typedef std::unordered_map<GLenum, GLuint> TextureParameterMap;
		typedef std::unordered_map<GLenum, bool> BooleanStateMap;
		struct TextureUnitParams {
			~TextureUnitParams() {
				tex_parameter_map.clear();
			}
			
			TextureParameterMap tex_parameter_map;
		};
		typedef std::unordered_map<GLuint, TextureUnitParams> TextureUnitMap;
		
		BindBufferMap bind_buffer_map_;
		TextureUnitMap texture_unit_map_;
		BooleanStateMap boolean_state_map_;
	};
	//--------------------------------------------------------------------------------------------------------------------------------
	void GLStateCacheManager::unregisterContext(intptr_t id) {
		auto iter = cache_map_.find(id);
		if (iter != cache_map_.end()) {
			if (iter->second == imp_)
				imp_ = nullptr;
			delete iter->second;
			cache_map_.erase(iter);
		}
		// imp_ always points to a non-empty GLStateCacheManagerImp pointer.
		if (imp_ == nullptr) {
			if (cache_map_.empty())
				cache_map_.insert(CacheMap::value_type(0, new GLStateCacheManagerImp()));
			imp_ = cache_map_.begin()->second;
		}
	}
	void GLStateCacheManager::switchContext(intptr_t id) {
		auto iter = cache_map_.find(id);
		if (iter != cache_map_.end())
			imp_ = iter->second;
		else {
			// Creates a new cache if not found.
			imp_ = new GLStateCacheManagerImp();
			imp_->initializeCache();
			cache_map_[id] = imp_;
		}
	}
	void GLStateCacheManager::clearCache() {
		imp_->clearCache();
	}
	void GLStateCacheManager::bindGLBuffer(GLenum target, GLuint buffer, bool force) {
		imp_->bindGLBuffer(target, buffer, force);
	}
	void GLStateCacheManager::deleteGLBuffer(GLenum target, GLuint buffer, bool force) {
		imp_->deleteGLBuffer(target, buffer, force);
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	void GLStateCacheManagerImp::initializeCache() {
		
	}
	void GLStateCacheManagerImp::clearCache() {
		
	}
	void GLStateCacheManagerImp::bindGLBuffer(GLenum target, GLuint buffer, bool force) {
		bool update = false;
		auto iter = bind_buffer_map_.find(target);
		if (iter == bind_buffer_map_.end()) {
			bind_buffer_map_.insert(BindBufferMap::value_type(target, buffer));
			update = true;
		}
		else if (iter->second != buffer) {
			iter->second = buffer;
			update = true;
		}
		if (update || force) {
			if (target == GL_FRAMEBUFFER) {
				glBindFramebuffer(target, buffer);
			}
			else if (target == GL_RENDERBUFFER) {
				glBindRenderbuffer(target, buffer);
			}
			else {
				glBindBuffer(target, buffer);
			}
		}
	}
	void GLStateCacheManagerImp::deleteGLBuffer(GLenum target, GLuint buffer, bool force) {
		if (buffer == 0)
			return;
		auto iter = bind_buffer_map_.find(target);
		if (iter != bind_buffer_map_.end() && (iter->second == buffer || force)) {
			if (target == GL_FRAMEBUFFER) {
				glDeleteFramebuffers(1, &buffer);
			}
			else if (target == GL_RENDERBUFFER) {
				glDeleteRenderbuffers(1, &buffer);
			}
			else {
				glDeleteBuffers(1, &buffer);
			}
			iter->second = 0;
		}
	}
}

#endif // AsteroGLStateCacheManager_h
