//
//  AsteroMeshLoader.h
//  Astero
//
//  Created by Yuzhe Wang on 8/26/17.
//  Copyright © 2017 Yuzhe Wang. All rights reserved.
//

#ifndef AsteroMeshLoader_h
#define AsteroMeshLoader_h

#include <fstream>
#include <memory>
#include "AsteroDataStream.h"
#include "AsteroMesh.h"

namespace Astero {
	typedef std::shared_ptr<std::fstream> FileStreamPtr;
	class Vector3;
	class Quaternion;
	
	class Loader {
	public:
		Loader();
		virtual ~Loader();
		
		enum Endian {
			ENDIAN_NATIVE,
			ENDIAN_BIG,
			ENDIAN_LITTLE
		};
		
	protected:
		virtual void writeFileHeader();
		virtual void writeChunkHeader(unsigned short id, size_t size);
		void writeFloats(const float * pfloat, size_t count);
		void writeShorts(const short * pshort, size_t count);
		void writeInts(const int * pint, size_t count);
		void writeBools(const bool * pbool, size_t count);
		void writeObject(const Vector3 & vector);
		void writeObject(const Quaternion & quaternion);
		void writeString(const std::string & string);
		void writeData(const void * const buffer, size_t size, size_t count);
		
		virtual void readFileHeader(FileStreamPtr & fstream_ptr);
		virtual unsigned short readChunk(FileStreamPtr& stream);
		void readFloats(FileStreamPtr & fstream_ptr, float * pdest, size_t count);
		void readShorts(FileStreamPtr & fstream_ptr, short * pdest, size_t count);
		void readInts(FileStreamPtr & fstream_ptr, int * pdest, size_t count);
		void readBool(FileStreamPtr & fstream_ptr, bool * pdest, size_t count);
		void readObject(FileStreamPtr & fstream_ptr, Vector3 & pdest);
		void readObject(FileStreamPtr & fstream_ptr, Quaternion & pdest);
		std::string readString(FileStreamPtr & fstream_ptr);
		
		std::shared_ptr<std::fstream> fstream_ptr_;
		std::string version;
		bool flip_endian_;
	};
	
	class Mesh;
	class MeshLoader {
	public:
		MeshLoader();
		virtual ~MeshLoader();
		
		void exportMesh(const Mesh * pmesh, const std::string & filename);
		void importMesh(FileStreamPtr & fstream_ptr, Mesh * pdest);
		void importMesh(DataStreamPtr & fstream_ptr, Mesh * pdest);
	protected:
		
	};
}

#endif // AsteroMeshLoader_h
