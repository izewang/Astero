//
//  AsteroHardwareBuffer.h
//  Astero
//
//  Created by Yuzhe Wang on 9/9/17.
//  Copyright © 2017 Yuzhe Wang. All rights reserved.
//

#ifndef AsteroHardwareBuffer_h
#define AsteroHardwareBuffer_h

#include <GL/glew.h>
#include <OpenGL/glu.h>

#include "AsteroPrerequisites.h"

namespace Astero {
	class HardwareBuffer {
	public:
		enum Usage {
			HBU_STATIC = 1,
			HBU_DYNAMIC = 2,
			HBU_WRITE_ONLY = 4,
			HBU_DISCARDABLE = 8,
			HBU_STATIC_WRITE_ONLY = 5,
			HBU_DYNAMIC_WRITE_ONLY = 6,
			HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE = 14
		};
		enum LockOption {
			HBL_NORMAL,
			HBL_DISCARD,
			HBL_READ_ONLY,
			HBL_NO_OVERWRITE,
			HBL_WRITE_ONLY
		};
		
		HardwareBuffer(Usage usage, bool use_system_memory, bool use_shadow_buffer);
		virtual ~HardwareBuffer();
		virtual void * lock(size_t offset, size_t size, LockOption option);
		void * lock(LockOption);
		virtual void unlock(void);
		virtual void readData(size_t offset, size_t size, void * dest) = 0;
		virtual void writeData(size_t offset, size_t size, const void * src,
							   bool discard_whole_buffer = false) = 0;
		virtual void copyData(HardwareBuffer & src_buffer, size_t src_offset,
							  size_t dest_offset, size_t size,
							  bool discard_whole_buffer = false);
		virtual void copyData(HardwareBuffer & src_buffer);
		virtual void updateFromShadow();
		size_t getSizeInBytes() const;
		Usage getUsage() const;
		bool hasShadowBuffer() const;
		bool isLocked() const;
	
	protected:
		size_t size_in_bytes_;
		Usage usage_;
		bool locked_;
		size_t lock_offset_;
		size_t lock_size_;
		bool use_system_memory_;
		bool use_shadow_buffer_;
		HardwareBuffer * shadow_buffer_;
		bool shadow_updated_;
		bool suppress_hardware_update_;
		
		virtual void * lockImpl(size_t offset, size_t size, LockOption option) = 0;
		virtual void unlockImpl(void) = 0;
	};
	
	template<typename T> struct HardwareBufferLockGuard {
		HardwareBufferLockGuard(const T & buffer, HardwareBuffer::LockOption option)
		: buffer_(buffer) {
			data_ = buffer_->lock(option);
		}
		HardwareBufferLockGuard(const T & buffer, size_t offset, size_t size, HardwareBuffer::LockOption option)
		: buffer_(buffer) {
			data_ = buffer_->lock(offset, size, option);
		}
		~HardwareBufferLockGuard() {
			buffer_->unlock();
		}
		const T & buffer_;
		void * data_;
	};
	//--------------------------------------------------------------------------------------------------------------------------------
	// HardwareVertexBuffer
	//--------------------------------------------------------------------------------------------------------------------------------
	// Abstact class for vertex buffer.
	class HardwareVertexBuffer : public HardwareBuffer {
	public:
		HardwareVertexBuffer(HardwareBufferManager * manager,
							 size_t vertex_size,
							 size_t vertex_num,
							 HardwareBuffer::Usage usage,
							 bool use_system_memory,
							 bool use_shadow_buffer);
		~HardwareVertexBuffer();
		HardwareBufferManager * getManager() const;
		size_t getVertexSize() const;
		size_t getVertexNum() const;
		bool getIsInstanceData() const;
		void setIsInstanceData(const bool val);
		unsigned int getInstanceDataStepRate() const;
		void setInstanceDataStepRate(const unsigned int val);
	protected:
		HardwareBufferManager * manager_;
		size_t vertex_size_;
		size_t vertex_num_;
		bool is_instance_data_;
		unsigned int instance_data_step_rate_;
	};
	
	typedef std::shared_ptr<HardwareVertexBuffer> HardwareVertexBufferPtr;
	typedef HardwareBufferLockGuard<HardwareVertexBufferPtr> HardwareVertexBufferLockGuard;
	
	//--------------------------------------------------------------------------------------------------------------------------------
	enum VertexElementSemantic {
		// Position, 3 reals per vertex.
		VES_POSITION = 1,
		// Blending weights.
		VES_BLEND_WEIGHTS = 2,
		// Blending indices.
		VES_BLEND_INDICES = 3,
		// Normal, 3 reals per vertex.
		VES_NORMAL = 4,
		// Diffuse colors.
		VES_DIFFUSE = 5,
		// Specular colors.
		VES_SPECULAR = 6,
		// Texture coordinates.
		VES_TEXTURE_COORDINATES = 7,
		// Binormal.
		VES_BINORMAL = 8,
		// Tangent.
		VES_TANGENT = 9,
		VES_COUNT = 9
	};
	
	// Vertex element type, used to identify the base types of the vertex contents
	enum VertexElementType
	{
		VET_FLOAT1 = 0,
		VET_FLOAT2 = 1,
		VET_FLOAT3 = 2,
		VET_FLOAT4 = 3,
		// alias to more specific colour type.
		VET_COLOUR = 4,
		VET_SHORT1 = 5,
		VET_SHORT2 = 6,
		VET_SHORT3 = 7,
		VET_SHORT4 = 8,
		VET_UBYTE4 = 9,
		// D3D style compact color.
		VET_COLOUR_ARGB = 10,
		// GL style compact color.
		VET_COLOUR_ABGR = 11,
		VET_DOUBLE1 = 12,
		VET_DOUBLE2 = 13,
		VET_DOUBLE3 = 14,
		VET_DOUBLE4 = 15,
		VET_USHORT1 = 16,
		VET_USHORT2 = 17,
		VET_USHORT3 = 18,
		VET_USHORT4 = 19,
		VET_INT1 = 20,
		VET_INT2 = 21,
		VET_INT3 = 22,
		VET_INT4 = 23,
		VET_UINT1 = 24,
		VET_UINT2 = 25,
		VET_UINT3 = 26,
		VET_UINT4 = 27
	};
	//--------------------------------------------------------------------------------------------------------------------------------
	class VertexElement {
	public:
		VertexElement() = default;
		VertexElement(unsigned short source, size_t offset, VertexElementType type, VertexElementSemantic semantic, unsigned short index = 0);
		
		unsigned short getSource() const;
		size_t getOffset() const;
		VertexElementType getType() const;
		VertexElementSemantic getSemantic() const;
		unsigned short getIndex() const;
		size_t getSize() const;
		size_t getTypeSize(VertexElementType type) const;
		static unsigned short getTypeCount(VertexElementType type);
		inline void baseVertexPointerToElement(void * base, void ** element) {
			*element = static_cast<void *>(static_cast<unsigned char *>(base) + offset_);
		}
	protected:
		unsigned short source_;
		size_t offset_;
		VertexElementType type_;
		VertexElementSemantic semantic_;
		unsigned short index_;
	};
	//--------------------------------------------------------------------------------------------------------------------------------
	VertexElement::VertexElement(unsigned short source,
								 size_t offset,
								 VertexElementType type,
								 VertexElementSemantic semantic,
								 unsigned short index)
	: source_(source), offset_(offset), type_(type), semantic_(semantic), index_(index) {
		
	}
	
	unsigned short VertexElement::getSource() const {
		return source_;
	}
	
	size_t VertexElement::getOffset() const {
		return offset_;
	}
	
	VertexElementType VertexElement::getType() const {
		return type_;
	}
	
	VertexElementSemantic VertexElement::getSemantic() const {
		return semantic_;
	}
	
	unsigned short VertexElement::getIndex() const {
		return index_;
	}
	
	size_t VertexElement::getSize() const {
		return getTypeSize(type_);
	}
	
	size_t VertexElement::getTypeSize(VertexElementType type) const {
		switch(type) {
			case VET_COLOUR:
			case VET_COLOUR_ARGB:
			case VET_COLOUR_ABGR:
				return sizeof(unsigned int);
			case VET_FLOAT1:
				return sizeof(float);
			case VET_FLOAT2:
				return sizeof(float) * 2;
			case VET_FLOAT3:
				return sizeof(float) * 3;
			case VET_FLOAT4:
				return sizeof(float) * 4;
			case VET_DOUBLE1:
				return sizeof(double);
			case VET_DOUBLE2:
				return sizeof(double) * 2;
			case VET_DOUBLE3:
				return sizeof(double) * 3;
			case VET_DOUBLE4:
				return sizeof(double) * 4;
			case VET_SHORT1:
			case VET_USHORT1:
				return sizeof(short);
			case VET_SHORT2:
			case VET_USHORT2:
				return sizeof(short) * 2;
			case VET_SHORT3:
			case VET_USHORT3:
				return sizeof(short) * 3;
			case VET_SHORT4:
			case VET_USHORT4:
				return sizeof(short) * 4;
			case VET_INT1:
			case VET_UINT1:
				return sizeof(int);
			case VET_INT2:
			case VET_UINT2:
				return sizeof(int) * 2;
			case VET_INT3:
			case VET_UINT3:
				return sizeof(int) * 3;
			case VET_INT4:
			case VET_UINT4:
				return sizeof(int) * 4;
			case VET_UBYTE4:
				return sizeof(unsigned char) * 4;
		}
		return 0;
	}
	
	unsigned short VertexElement::getTypeCount(VertexElementType type) {
		switch (type)
		{
			case VET_COLOUR:
			case VET_COLOUR_ABGR:
			case VET_COLOUR_ARGB:
			case VET_FLOAT1:
			case VET_SHORT1:
			case VET_USHORT1:
			case VET_UINT1:
			case VET_INT1:
			case VET_DOUBLE1:
				return 1;
			case VET_FLOAT2:
			case VET_SHORT2:
			case VET_USHORT2:
			case VET_UINT2:
			case VET_INT2:
			case VET_DOUBLE2:
				return 2;
			case VET_FLOAT3:
			case VET_SHORT3:
			case VET_USHORT3:
			case VET_UINT3:
			case VET_INT3:
			case VET_DOUBLE3:
				return 3;
			case VET_FLOAT4:
			case VET_SHORT4:
			case VET_USHORT4:
			case VET_UINT4:
			case VET_INT4:
			case VET_DOUBLE4:
			case VET_UBYTE4:
				return 4;
		}
		return 0;
	}
	//--------------------------------------------------------------------------------------------------------------------------------
	class VertexDeclaration {
	public:
		typedef std::list<VertexElement> VertexElementList;
		static bool vertexElementLess(const VertexElement & lhs, const VertexElement & rhs);
		
		VertexDeclaration();
		virtual ~VertexDeclaration();
		
		const VertexElementList getElements() const;
		void sort();
		virtual const VertexElement & addElement();
		virtual const VertexElement & insertElement();
	protected:
		VertexElementList vertex_element_list_;
	};
	
	class VertexBufferBinding {
	public:
		typedef std::unordered_map<unsigned short, HardwareVertexBufferPtr> VertexBufferBindingMap;
		typedef std::unordered_map<unsigned short, unsigned short> BindingIndexMap;
		
		VertexBufferBinding();
		~VertexBufferBinding();
		
		virtual void setBinding();
		virtual void unsetBinding();
		virtual void unsetAllBindings();
		virtual const VertexBufferBindingMap & getBindings() const;
		virtual const HardwareVertexBufferPtr & getBuffer(unsigned short index) const;
		virtual bool isBufferBound(unsigned short index) const;
		// Indicates whether this binding has an element which contains instance data.
		virtual bool getHasInstanceData() const;
	protected:
		VertexBufferBindingMap vertex_buffer_binding_map_;
		
	};
	//--------------------------------------------------------------------------------------------------------------------------------
	class DefaultHardwareVertexBuffer : public HardwareVertexBuffer {
	public:
		DefaultHardwareVertexBuffer(size_t vertex_size,
									size_t vertex_num,
									HardwareBuffer::Usage usage);
		~DefaultHardwareVertexBuffer();
		
		void * lock(size_t offset, size_t size, LockOption option) override;
		void unlock()  override;
		void readData(size_t offset, size_t size, void * dest) override;
		void writeData(size_t offset, size_t size, const void * src, bool discard_whole_buffer = false) override;
		
	protected:
		unsigned char * data_;
		void * lockImpl(size_t offset, size_t size, LockOption option) override;
		void unlockImpl() override;
	};
	
} // namespace Astero

namespace Astero {

	//--------------------------------------------------------------------------------------------------------------------------------
	class GLHardwareVertexBuffer : public HardwareVertexBuffer {
	public:
		GLHardwareVertexBuffer(HardwareBufferManager * manager,
							   size_t vertex_size,
							   size_t vertex_num,
							   HardwareBuffer::Usage usage,
							   bool use_shadow_buffer);
		~GLHardwareVertexBuffer();
		
		GLuint getGLBufferId() const;
		void readData(size_t offset, size_t size, void * dest) override;
		void writeData(size_t offset, size_t size, const void * source,
					   bool discard_whole_buffer = false) override;
		void updateFromShadow() override;
		
	protected:
		void * lockImpl(size_t offset, size_t size, LockOption option) override;
		void unlockImpl() override;

	private:
		GLuint buffer_id_;
		bool locked_to_scratch_;
		bool scratch_upload_on_unlock_;
		size_t scratch_offset_;
		size_t scratch_size_;
		void * scratch_;
	};

} // namespace Astero
//--------------------------------------------------------------------------------------------------------------------------------
// HardwareIndexBuffer
//--------------------------------------------------------------------------------------------------------------------------------
namespace Astero {
	class HardwareIndexBuffer : public HardwareBuffer {
	public:
		enum IndexType {
			IT_16BIT,
			IT_32BIT
		};
		HardwareIndexBuffer(HardwareBufferManager * manager,
							IndexType index_type,
							size_t index_num,
							bool use_shadow_buffer);
		IndexType getType() const {
			return index_type_;
		}
		
		size_t getIndexSize() const {
			return index_size_;
		}
		
	protected:
		IndexType index_type_;
		size_t index_size_;
	};
	class GLHardwareIndexBuffer : public HardwareIndexBuffer {
	public:
		GLHardwareIndexBuffer(HardwareBufferManager * manager,
							  IndexType index_type,
							  size_t index_num,
							  HardwareBuffer::Usage usage,
							  bool use_shadow_buffer);
		virtual void readData(size_t offset, size_t size, void * dest) {
			
		}
		virtual void writeData(size_t offset, size_t size, const void * source,
							   bool discard_whole_buffer = false) {
			
		}
		virtual void copyData(HardwareBuffer & src_buffer, size_t src_offset,
							  size_t dest_offset, size_t size,
							  bool discard_whole_buffer = false) {
			
		}
		GLuint getGLBufferId() const {return buffer_id_;}
	protected:
		virtual void * lockImpl(size_t offset, size_t size, LockOption option) {
			return nullptr;
		}
		virtual void unlockImpl(void) {
			
		}
	private:
		GLuint buffer_id_;
	};
	
	class DefaultHardwareIndexBuffer : public HardwareIndexBuffer
	{
	public:
		DefaultHardwareIndexBuffer(IndexType index_type, size_t index_num, HardwareBuffer::Usage usage);
		~DefaultHardwareIndexBuffer();
		void readData(size_t offset, size_t size, void * dest) override;
		void writeData(size_t offset, size_t size, const void * source, bool discard_whole_buffer = false) override;
		void * lock(size_t offset, size_t size, LockOption option) override;
		void unlock() override;
		
	protected:
		void * lockImpl(size_t offset, size_t size, LockOption option) override;
		void unlockImpl() override;
		unsigned char * data_;

	};
	
} // namespace Astero
//--------------------------------------------------------------------------------------------------------------------------------
// RenderToVertexBuffer
//--------------------------------------------------------------------------------------------------------------------------------
#include "AsteroRenderOperation.h"

namespace Astero {
	typedef std::shared_ptr<Material> MaterialPtr;
	class VertexData;
	
	class RenderToVertexBuffer {
	public:
		RenderToVertexBuffer();
		virtual ~RenderToVertexBuffer();
		
		VertexDeclaration * getVertexDeclaration();
		unsigned int getMaxVertexCount() const;
		void setMaxVertexCount(unsigned int maxVertexCount);
		RenderOperation::OperationType getOperationType() const;
		void setOperationType(RenderOperation::OperationType operation_type);
		
		virtual void getRenderOperation(RenderOperation & render_operation) = 0;
		virtual void update(SceneManager * scene_manager) = 0;
		virtual void reset();
		void setSourceRenderable(Renderable * source);
		const Renderable * getSourceRenderable() const;
		const MaterialPtr & getRenderToBufferMaterial();
		void setRenderToBufferMaterialName(const std::string & material_name);
	protected:
		RenderOperation::OperationType operation_type_;
		MaterialPtr material_;
		Renderable * source_renderable_;
		VertexData * vertex_data_;
		unsigned int max_vertex_count_;
	};
	
	class GLRenderToVertexBuffer : public RenderToVertexBuffer {
		void getRenderOperation(RenderOperation & render_operation);
		void update(SceneManager * scene_manager);
	};
} // namespace Astero

//--------------------------------------------------------------------------------------------------------------------------------
// HardwarePixelBuffer
//--------------------------------------------------------------------------------------------------------------------------------
namespace Astero {
	class HardwarePixelBuffer : public HardwareBuffer {
		
	};
	
	class GLHardwarePixelBuffer : public HardwarePixelBuffer {
		
	};
	
	class GLTextureBuffer : public GLHardwarePixelBuffer {
		
	};
	
	class GLRenderBuffer : public GLHardwarePixelBuffer {
		
	};
}

#endif // AsteroHardwareBuffer_h
