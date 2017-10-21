//
//  AsteroRenderSystem.cpp
//  Astero
//
//  Created by Yuzhe Wang on 10/7/17.
//  Copyright © 2017 Yuzhe Wang. All rights reserved.
//

#include <stdio.h>
#include <algorithm>

#include "AsteroRenderSystem.h"
#include "AsteroHardwareBuffer.h"
#include "AsteroRenderOperation.h"
#include "AsteroGLStateCacheManager.h"
#include "AsteroGpuProgram.h"
#include "AsteroHardwareBufferManager.h"
#include "AsteroRenderSystemCapabilities.h"
#include "AsteroRenderOperation.h"
#include "AsteroGLDefaultHardwareBuffer.h"

namespace Astero {
	
	//--------------------------------------------------------------------------------------------------------------------------------
	bool RenderSystem::updatePassIterationRenderState() {
		if (current_pass_iteration_count_ <= 1)
			return false;
		--current_pass_iteration_count_;
		++current_pass_iteration_number_;
		// Do something.
		return true;
	}
	
	GLRenderSystem::GLRenderSystem() {
		state_cache_manager_ = new GLStateCacheManager;
		render_attribs_bound_.reserve(100);
		render_instance_attribs_bound_.reserve(100);
		
		
	}
	
	RenderSystemCapabilities* GLRenderSystem::createRenderSystemCapabilities() const {
		RenderSystemCapabilities * rsc = new RenderSystemCapabilities();
		return rsc;
	}
	
	const std::string & GLRenderSystem::getName() const {
		static std::string name("OpenGL Render System");
		return name;
	}

	void GLRenderSystem::render(const RenderOperation & operation) {
		// Call super class
		RenderSystem::render(operation);
		if (!enable_fixed_pipeline_
			&& !real_capabilities_->hasCapability(RSC_FIXED_FUNCTION)
			&& (current_vertex_program_ == nullptr
				|| (current_fragment_program_ == nullptr && operation.operation_type != RenderOperation::OT_POINT_LIST))) {
			// Attempted to render using the fixed pipeline when it is disabled.
			assert(false);
		}
		// Reset max built-in texture attrib index.
		max_built_in_texture_attrib_index_ = 0;
		// Has global instance data or operation has instance data.
		bool has_instance_data = (operation.use_global_instance_vertex_buffer
								  && global_instance_vertex_buffer_ != nullptr
								  && global_instance_vertex_buffer_vertex_declaration_ != nullptr) || operation.vertex_data->vertex_buffer_binding->getHasInstanceData();
		// Calculates instance number.
		unsigned int instance_number = operation.instance_number;
		if (operation.use_global_instance_vertex_buffer)
			instance_number *= global_instance_number_;
		// Binds vertex element to GPU.
		for (auto & element : operation.vertex_data->vertex_declaration->getElements()) {
			size_t source = element.getSource();
			// Skips unbounded elements.
			if (!operation.vertex_data->vertex_buffer_binding->isBufferBound(source))
				continue;
			HardwareVertexBufferPtr vertex_buffer = operation.vertex_data->vertex_buffer_binding->getBuffer(source);
			bindVertexElementToGpu(element, vertex_buffer, operation.vertex_data->vertex_start,
								   render_attribs_bound_, render_instance_attribs_bound_);
		}
		// Binds global instance vertex element to GPU.
		if (global_instance_vertex_buffer_ != nullptr && global_instance_vertex_buffer_vertex_declaration_ != nullptr) {
			for (auto & element : global_instance_vertex_buffer_vertex_declaration_->getElements()) {
				bindVertexElementToGpu(element, global_instance_vertex_buffer_, 0,
									   render_attribs_bound_, render_instance_attribs_bound_);
			}
		}
		// OpenGL modes.
		GLint prim_type;
		bool use_adjacency = geometry_program_bound_ && current_geometry_program_ && current_geometry_program_->isAdjacencyInfoRequired();
		switch (operation.operation_type) {
			case RenderOperation::OT_POINT_LIST:
				prim_type = GL_POINTS;
				break;
			case RenderOperation::OT_LINE_LIST:
				prim_type = use_adjacency ? GL_LINES_ADJACENCY : GL_LINES;
				break;
			case RenderOperation::OT_LINE_STRIP:
				prim_type = use_adjacency ? GL_LINE_STRIP_ADJACENCY : GL_LINE_STRIP;
				break;
			default:
			case RenderOperation::OT_TRIANGLE_LIST:
				prim_type = use_adjacency ? GL_TRIANGLES_ADJACENCY : GL_TRIANGLES;
				break;
			case RenderOperation::OT_TRIANGLE_STRIP:
				prim_type = use_adjacency ? GL_TRIANGLE_STRIP_ADJACENCY : GL_TRIANGLE_STRIP;
				break;
			case RenderOperation::OT_TRIANGLE_FAN:
				prim_type = GL_TRIANGLE_FAN;
				break;
		}
		// glClentActiveTexture.
		bool multitexturing = (current_capabilities_->getTextureUnitNumber() > 1);
		if (multitexturing)
			glClientActiveTexture(GL_TEXTURE0);
		// glDrawElements or glDrawElementsInstanced.
		if (operation.use_indices) {
			// Pointer to the location where indices data stored.
			void * buffer_data = nullptr;
			if (current_capabilities_->hasCapability(RSC_VBO)) {
				state_cache_manager_->bindGLBuffer(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLHardwareIndexBuffer *>(operation.index_data->index_buffer.get())->getGLBufferId());
				buffer_data = (char *)NULL + (operation.index_data->index_start * operation.index_data->index_buffer->getIndexSize());
			}
			else {
				buffer_data = static_cast<GLDefaultHardwareIndexBuffer *>(operation.index_data->index_buffer.get())->getData(operation.index_data->index_start * operation.index_data->index_buffer->getIndexSize());
			}
			GLenum index_type = (operation.index_data->index_buffer->getType() == HardwareIndexBuffer::IT_16BIT) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
			do {
				// Updates derived depth bias.
				if (derived_depth_bias_ && current_pass_iteration_number_ > 0) {
					setDepthBias(derived_depth_bias_base_ + derived_depth_bias_multiplier_ * current_pass_iteration_number_,
								 derived_depth_bias_slope_scale_);
				}
				if(has_instance_data) {
					glDrawElementsInstanced(prim_type, operation.index_data->index_count, index_type, buffer_data, instance_number);
				}
				else {
					glDrawElements(prim_type, operation.index_data->index_count, index_type, buffer_data);
				}
			} while (updatePassIterationRenderState());
		}
		// glDrawArrays or glDrawArraysInstanced.
		else {
			do {
				if (derived_depth_bias_ && current_pass_iteration_number_ > 0) {
					setDepthBias(derived_depth_bias_base_ + derived_depth_bias_multiplier_ * current_pass_iteration_number_,
								 derived_depth_bias_slope_scale_);
				}
				if (has_instance_data) {
					glDrawArraysInstanced(prim_type, 0, operation.vertex_data->vertex_count, instance_number);
				}
				else {
					glDrawArrays(prim_type, 0, operation.vertex_data->vertex_count);
				}
			} while (updatePassIterationRenderState());
		}
		// Disable client states which have been enabled in bindVertexElementToGpu.
		glDisableClientState(GL_VERTEX_ARRAY);
		if (multitexturing) {
			unsigned short enabled_texture_number = std::max(std::min(texture_units_disabled_from_, fixed_function_texture_units_number_), (unsigned short)(max_built_in_texture_attrib_index_ + 1));
			for (unsigned short i = 0; i < enabled_texture_number; ++i) {
				glClientActiveTexture(GL_TEXTURE0 + i);
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			}
			glClientActiveTexture(GL_TEXTURE0);
		}
		else {
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		if (GLEW_EXT_secondary_color) {
			glDisableClientState(GL_SECONDARY_COLOR_ARRAY);
		}
		for (auto index : render_attribs_bound_) {
			glDisableVertexAttribArray(index);
		}
		for (auto index : render_instance_attribs_bound_) {
			glVertexAttribDivisor(index, 0);
		}
		render_attribs_bound_.clear();
		render_instance_attribs_bound_.clear();
	}
	void GLRenderSystem::bindVertexElementToGpu(const VertexElement & element, HardwareVertexBufferPtr vertex_buffer,
								const size_t vertex_start, std::vector<GLuint> & attribs_bound,
								std::vector<GLuint> & instance_attribs_bound) {
		// Retrieves buffer_data.
		void * buffer_data = nullptr;
		const GLHardwareVertexBuffer * gl_vertex_buffer = static_cast<const GLHardwareVertexBuffer *>(vertex_buffer.get());
		if (current_capabilities_->hasCapability(RSC_VBO)) {
			state_cache_manager_->bindGLBuffer(GL_ARRAY_BUFFER, gl_vertex_buffer->getGLBufferId());
			buffer_data = (char *)NULL + element.getOffset();
		}
		else {
			buffer_data = static_cast<const GLDefaultHardwareVertexBuffer *>(vertex_buffer.get())->getData(element.getOffset());
		}
		VertexElementSemantic semantic = element.getSemantic();
		bool multitexturing = current_capabilities_->getTextureUnitNumber() > 1;
		bool is_custom_attrib = false;
		if (current_vertex_program_ != nullptr) {
			is_custom_attrib = current_vertex_program_->isAttributeValid(semantic, element.getIndex());
			if (gl_vertex_buffer->getIsInstanceData()) {
				GLint attrib = current_vertex_program_->getAttributeIndex(semantic, element.getIndex());
				glVertexAttribDivisor(attrib, gl_vertex_buffer->getInstanceDataStepRate());
				instance_attribs_bound.push_back(attrib);
			}
		}
		// Custom attribut support.
		if (is_custom_attrib) {
			GLint attrib = current_vertex_program_->getAttributeIndex(semantic, element.getIndex());
			unsigned short type_count = VertexElement::getTypeCount(element.getType());
			GLboolean normalized = GL_FALSE;
			switch (element.getType()) {
				case VET_COLOUR:
				case VET_COLOUR_ABGR:
				case VET_COLOUR_ARGB:
					type_count = 4;
					normalized = GL_TRUE;
					break;
				default:
					break;
			}
			glVertexAttribPointer(attrib, type_count, GLHardwareBufferManager::getGLType(element.getType()), normalized, static_cast<GLsizei>(vertex_buffer->getVertexSize()), buffer_data);
			glEnableVertexAttribArray(attrib);
			attribs_bound.push_back(attrib);
		}
		// Fixed-function & built in attribute support.
		else {
			switch (semantic) {
				case VES_POSITION:
					glVertexPointer(VertexElement::getTypeCount(element.getType()), GLHardwareBufferManager::getGLType(element.getType()), static_cast<GLsizei>(vertex_buffer->getVertexSize()), buffer_data);
					glEnableClientState(GL_VERTEX_ARRAY);
					break;
				case VES_NORMAL:
					glNormalPointer(GLHardwareBufferManager::getGLType(element.getType()), static_cast<GLsizei>(vertex_buffer->getVertexSize()), buffer_data);
					glEnableClientState(GL_NORMAL_ARRAY);
					break;
				case VES_DIFFUSE:
					glColorPointer(4, GLHardwareBufferManager::getGLType(element.getType()), static_cast<GLsizei>(vertex_buffer->getVertexSize()), buffer_data);
					glEnableClientState(GL_COLOR_ARRAY);
					break;
				case VES_SPECULAR:
					if (GLEW_EXT_secondary_color) {
						glSecondaryColorPointer(4, GLHardwareBufferManager::getGLType(element.getType()), static_cast<GLsizei>(vertex_buffer->getVertexSize()), buffer_data);
						glEnableClientState(GL_SECONDARY_COLOR_ARRAY);
					}
					break;
				case VES_TEXTURE_COORDINATES:
					if (current_vertex_program_) {
						glClientActiveTexture(GL_TEXTURE0 + element.getIndex());
						glTexCoordPointer(VertexElement::getTypeCount(element.getType()), GLHardwareBufferManager::getGLType(element.getType()), static_cast<GLsizei>(vertex_buffer->getVertexSize()), buffer_data);
						glEnableClientState(GL_TEXTURE_COORD_ARRAY);
						// Updates max built-in texture attrib index.
						if (element.getIndex() > max_built_in_texture_attrib_index_)
							max_built_in_texture_attrib_index_ = element.getIndex();
					}
					else {
						// DisabledTexUnitsFrom
						for (unsigned int i = 0; i < texture_units_disabled_from_; ++i) {
							if (texture_coordinate_index_[i] == element.getIndex() && i < fixed_function_texture_units_number_) {
								if (multitexturing)
									glClientActiveTexture(GL_TEXTURE0 + i);
								glTexCoordPointer(VertexElement::getTypeCount(element.getType()), GLHardwareBufferManager::getGLType(element.getType()), static_cast<GLsizei>(vertex_buffer->getVertexSize()), buffer_data);
								glEnableClientState(GL_TEXTURE_COORD_ARRAY);
							}
						}
					}
					break;
				default:
					break;
			}
		}
	}
} // namespace Astero
