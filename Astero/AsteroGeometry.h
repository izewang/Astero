//
//  AsteroGeometry.h
//  Astero
//
//  Created by Yuzhe Wang on 8/10/17.
//  Copyright © 2017 Yuzhe Wang. All rights reserved.
//

#ifndef AsteroGeometry_h
#define AsteroGeometry_h

namespace Astero {
	// 2-dimensional vector
	class Vector2 {
	public:
		// constructor/destructor
		inline Vector2() = default;
		inline explicit Vector2(const float x_, const float y_) : x(x_), y(y_) {}
		inline explicit Vector2(const float scaler) : x(scaler), y(scaler) {}
		inline explicit Vector2(const float coordinate[2]) : x(coordinate[0]), y(coordinate[1]) {}
		inline explicit Vector2(float * const coordinate) : x(coordinate[0]), y(coordinate[1]) {}
		// operator overload
		
		// public data member
	public:
		float x, y;
	};
	
	// 3-dimensional vector
	class Vector3 {
	public:
		// constructor/destructor
		inline Vector3() = default;
		inline explicit Vector3(const float x_, const float y_, const float z_) : x(x_), y(y_), z(z_) {}
		
		// public data member
	public:
		float x, y, z;
	};
	
	// 4-dimensional vector
	class Vector4 {
	public:
		// constructor/destructor
		inline Vector4() = default;
		inline explicit Vector4(const float x_, const float y_, const float z_, const float w_)
		: x(x_), y(y_), z(z_), w(w_) {}
		
		// public data member
	public:
		float x, y, z, w;
	};
	
	class Matrix3 {
	public:
		float m[3][3];
	};
	
	class Matrix4 {
	public:
		float m[4][4];
	};
	
	class Quaternion {
		float w, x, y, z;
	};
	
	class Plane {
		Vector3 normal;
		float d;
	};
	
} // namespace Astero

#endif // AsteroGeometry_h
