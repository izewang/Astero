//
//  AsteroRenderSystem.h
//  Astero
//
//  Created by Yuzhe Wang on 8/27/17.
//  Copyright © 2017 Yuzhe Wang. All rights reserved.
//

#ifndef AsteroRenderSystem_h
#define AsteroRenderSystem_h

#include <GL/glew.h>
#include <OpenGL/glu.h>
#include "AsteroRenderOperation.h"

namespace Astero {
	class DepthBuffer;
	typedef std::vector<DepthBuffer *> DepthBufferVector;
	typedef std::unordered_map<unsigned short, DepthBufferVector> DepthBufferVectorMap;
	class RenderTarget;
	typedef std::unordered_map<std::string, RenderTarget *> RenderTargetMap;
	typedef std::multimap<unsigned char, RenderTarget *> RenderTargetPriorityMap;
	class TextureManager;
	class Viewport;
	enum CullingMode {
		
	};
	class Plane;
	typedef std::list<Plane> PlaneList;
	class ConfigOption;
	typedef std::unordered_map<std::string, ConfigOption> ConfigOptionMap;
	enum ShadeOptions {
		
	};
	class Light;
	typedef std::list<Light> LightList;
	enum ColorValue {
		White,
		Black
	};
	class Texture;
	typedef std::shared_ptr<Texture> TexturePtr;
	enum FogMode {
		FOG_NONE
	};
	class VertexDeclaration;
	class VertexBufferBinding;

	class GpuProgram;
	typedef std::unordered_map<std::string, std::string> NameValuePairList;
	
	class RenderSystem {
	public:
		RenderSystem();
		virtual ~RenderSystem();
		
		virtual const std::string & getName() const = 0;
		virtual ConfigOptionMap & getConfigOptions() = 0;
		virtual void setConfigOption(const std::string & name, const std::string & value) = 0;
		virtual RenderWindow* initialize(bool auto_create_window, const std::string & window_title = "Astero Render Window");
		virtual RenderSystemCapabilities* createRenderSystemCapabilities() const = 0;
		virtual void shutdown();
		virtual void setAmbientLight(float r, float g, float b) = 0;
		virtual void setShadingType(ShadeOptions so) = 0;
		// Creates a new rendering window, specified by parameters.
		virtual RenderWindow * createRenderWindow(const std::string & name, unsigned int width, unsigned int height, bool full_screen,
												  const NameValuePairList * misc_params = 0) = 0;
		virtual void destroyRenderWindow(const std::string & name);
		virtual void destroyRenderTexture(const std::string & name);
		virtual void destroyRenderTarget(const std::string & name);
		virtual void attachRenderTarget(RenderTarget & render_target);
		virtual RenderTarget * getRenderTarget(const std::string & name);
		virtual void setDepthBufferFor(RenderTarget * render_target);
		virtual void useLight(const LightList & lights, unsigned short limit) = 0;
		virtual void setWorldMatrix(const Matrix4 & world_matrix);
		virtual void setViewMatrix(const Matrix4 & view_matrix);
		virtual void setProjectionMatrix(const Matrix4 & projection_matrix);
		virtual void setSurfaceParameters(const ColorValue & ambient,
									  const ColorValue & diffuse,
									  const ColorValue & specular) = 0;
		virtual void setPointParameters(float size);
		virtual void setTexture(size_t unit, bool enabled, const TexturePtr & text_ptr) = 0;
		virtual void setVertexTexture(size_t unit, const TexturePtr & text_ptr);
		virtual void setGeometryTexture(size_t unit, const TexturePtr & text_ptr);
		virtual void setComputeTexture(size_t unit, const TexturePtr & text_ptr);
		virtual void setTesselationHullTexture(size_t unit, const TexturePtr & text_ptr);
		virtual void setTesselationDomainTexture(size_t unit, const TexturePtr & text_ptr);
		virtual void setTextureCoordinateSet(size_t unit, size_t index) = 0;
		virtual void cleanupDepthBuffers();
		virtual void beginFrame() = 0;
		virtual void endFrame() = 0;
		virtual void setViewport(Viewport * viewport) = 0;
		virtual Viewport * getViewport();
		virtual void setCullingMode(CullingMode mode) = 0;
		virtual CullingMode getCullingMode() const;
		virtual void setFog(FogMode mode = FOG_NONE,
							const ColorValue & color = ColorValue::White,
							float exp_density = 1.0f,
							float linear_start = 0.0f,
							float linear_end = 1.0f) = 0;
		virtual void beginGeometryCount();
		virtual unsigned int getFaceCount() const;
		virtual unsigned int getBatchCount() const;
		virtual unsigned int getVertexCount() const;
		virtual void setVertexDeclaration(VertexDeclaration * decl) = 0;
		virtual void setVertexBufferBinding(VertexBufferBinding * binding) = 0;
		virtual void render(const RenderOperation & operation);
		virtual void bindGpuProgram(GpuProgram* prg);
		virtual void setClipPlanes(const PlaneList & clip_planes);
		virtual void clearFrameBuffer(unsigned int buffers,
									  const ColorValue & colour = ColorValue::Black,
									  float depth = 1.0f) = 0;
		virtual void setRenderTarget(RenderTarget * render_target) = 0;
		virtual void registerThread() = 0;
		virtual void unregisterThread() = 0;
		virtual bool setDrawBuffer();
	protected:
		bool updatePassIterationRenderState();

		DepthBufferVectorMap depth_buffer_pool_;
		RenderTargetMap render_targets_;
		RenderTargetPriorityMap prioritized_render_targets_;
		RenderTarget * active_render_target_;
		TextureManager * texture_manager_;
		Viewport * active_viewport_;
		CullingMode culling_mode_;
		PlaneList clip_planes_;
		bool enable_fixed_pipeline_;
		RenderSystemCapabilities * real_capabilities_;
		RenderSystemCapabilities * current_capabilities_;
		HardwareVertexBufferPtr global_instance_vertex_buffer_;
		VertexDeclaration * global_instance_vertex_buffer_vertex_declaration_;
		unsigned int global_instance_number_;
		bool vertex_program_bound_;
		bool geometry_program_bound_;
		bool frament_program_bound_;
		// Indicates whether to update the depth bias per render call.
		bool derived_depth_bias_;
		float derived_depth_bias_base_;
		float derived_depth_bias_multiplier_;
		float derived_depth_bias_slope_scale_;
		// Number of times to render the current state.
		unsigned short current_pass_iteration_number_;
		unsigned short current_pass_iteration_count_;
		// Texture units disabled from this.
		unsigned short texture_units_disabled_from_;
	};
	
	class GLStateCacheManager;
	class GLGpuProgram;
} // namespace Astero

#include "AsteroGeometry.h"
#include "AsteroDepthBuffer.h"

namespace Astero {
	class GLRenderSystem : public RenderSystem {
	public:
		GLRenderSystem();
		
		// See RenderSystem.
		RenderWindow* initialize(bool auto_create_window, const std::string & window_title = "Astero Render Window") override;
		// See RenderSystem.
		RenderWindow * createRenderWindow(const std::string & name, unsigned int width, unsigned int height, bool full_screen,
										  const NameValuePairList * misc_params = 0) override;
		const std::string & getName() const override;
		virtual RenderSystemCapabilities * createRenderSystemCapabilities() const override;
		
		void render(const RenderOperation & operation) override;
		HardwareVertexBufferPtr getGlobalInstanceVertexBuffer();
		VertexDeclaration * getGlobalInstanceVertexBufferVertexDeclaration();
		void setDepthBias(float constant_bias, float slope_scale_bias);
		
	protected:
		void bindVertexElementToGpu(const VertexElement & element, HardwareVertexBufferPtr vertex_buffer,
									const size_t vertex_offset, std::vector<GLuint> & attribs_bound,
									std::vector<GLuint> & instance_attribs_bound);
		
	private:
		std::vector<GLuint> render_attribs_bound_;
		std::vector<GLuint> render_instance_attribs_bound_;
		GLStateCacheManager * state_cache_manager_;
		GLGpuProgram * current_vertex_program_;
		GLGpuProgram * current_fragment_program_;
		GLGpuProgram * current_geometry_program_;
		unsigned short max_built_in_texture_attrib_index_;
		unsigned short fixed_function_texture_units_number_;
		unsigned short texture_coordinate_index_[16];
		GLSupport * gl_support_;
		bool gl_initialized_;
		
		Matrix4 view_matrix_;
		Matrix4 world_matrix_;
		Matrix4 texture_matrix_;
	};
}

#endif // AsteroRenderSystem_h
