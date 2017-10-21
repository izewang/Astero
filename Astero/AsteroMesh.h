//
//  AsteroMesh.h
//  Astero
//
//  Created by Yuzhe Wang on 8/25/17.
//  Copyright © 2017 Yuzhe Wang. All rights reserved.
//

#ifndef AsteroMesh_h
#define AsteroMesh_h

#include <map>
#include <vector>
#include <unordered_map>
#include <memory>
#include "AsteroResource.h"
#include "AsteroMeshLoader.h"

namespace Astero {	
	// This struct holds a vertex index, a bone index, and weight representing assignment of a vertex to bone
	// for animation.
	struct VertexBoneAssignment {
		unsigned int vertex_index;
		unsigned short bone_index;
		float weight;
	};
	
	struct AxisAlignedBoundingBox;
	struct Sphere;
	
	class Mesh;
	class SubMesh {
	public:
		SubMesh(const std::string & name) : name_(name) {}
		Mesh * parent;
	protected:
		std::string name_;
	};
	class Skeleton;
	class Animation;
	class HardwareBuffer;
	class VertexData;
		
	typedef std::shared_ptr<Skeleton> SkeletonPtr;
	
	// Mesh resource class holds information not only of vertices and triangles, but also reference to materials
	// LOD information, skeleton/bone information, keyframe animation information. This class also strongly owned
	// submeshes.
	class Mesh : public Resource {
		friend class SubMesh;
		friend class MeshLoader;
		
	public:
		typedef std::multimap<size_t, VertexBoneAssignment> VertexBoneAssignmentList;
		typedef VertexBoneAssignmentList::iterator VertexBoneAssignmentIterator;
		typedef std::vector<SubMesh *> SubMeshList;
		typedef std::unordered_map<std::string, unsigned short> SubMeshNameMap;
		
		Mesh(Handle handle, const std::string & name, const std::string & group) : Resource(handle, name, group) {
			
		}
		~Mesh();
		
		SubMesh * createSubMesh(const std::string & name) {
			SubMesh * sub_mesh = new SubMesh(name);
			sub_mesh->parent = this;
			sub_mesh_list_.push_back(sub_mesh);
			sub_mesh_name_map_[name] = sub_mesh_list_.size() - 1;
			return sub_mesh;
		}
		void destroySubMesh(const std::string & name) {
			unsigned short index = sub_mesh_name_map_[name];
			sub_mesh_name_map_.erase(name);
			sub_mesh_list_.erase(sub_mesh_list_.begin() + index);
		}
		SubMesh * getSubMesh(unsigned short index) {
			assert(index < sub_mesh_list_.size());
			return sub_mesh_list_[index];
		}
		SubMesh * getSubMesh(const std::string & name) {
			unsigned short index = sub_mesh_name_map_[name];
			assert(index < sub_mesh_list_.size());
			return sub_mesh_list_[index];
		}
		float getBoundingSphereRadius() const {
			return bounding_sphere_radius_;
		}
		
		VertexData * shared_vertex_data_;
		
	protected:
		typedef std::unordered_map<std::string, Animation*> AnimationList;
		
		void prepareImpl() override {
			// Open resouce file.
			data_stream_ptr_ = ResourceGroupManager::getSingleton().openResource(name_, group_);
			
			// Read from hard disc to RAM.
			data_stream_ptr_ = std::shared_ptr<MemoryDataStream>(new MemoryDataStream(name_, data_stream_ptr_));
		}
		void unprepareImpl() override;
		void loadImpl() override {
			MeshLoader mesh_loader;
			DataStreamPtr data(data_stream_ptr_);
			data_stream_ptr_ = nullptr;
			mesh_loader.importMesh(data, this);
			//updateMaterialForAllSubMeshes();
		}
		void unloadImpl() override;
		void postLoadImpl() override;
		void calculateSize() override;
		
		SubMeshList sub_mesh_list_;
		SubMeshNameMap sub_mesh_name_map_;
		std::string skeleton_name_;
		SkeletonPtr skeleton_ptr_;
		VertexBoneAssignmentList vertex_bone_assignment_list_;
		AnimationList animation_list_;
		float bounding_sphere_radius_;
		DataStreamPtr data_stream_ptr_;
	};
}


#endif // AsteroMesh_h
