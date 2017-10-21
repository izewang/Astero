//
//  AsteroSingleton.tpp.h
//  Astero
//
//  Created by Yuzhe Wang on 10/4/17.
//  Copyright © 2017 Yuzhe Wang. All rights reserved.
//

#ifndef AsteroSingleton_tpp
#define AsteroSingleton_tpp

#include "AsteroPrerequisites.h"

namespace Astero {
	template <typename T>
	class Singleton {
	public:
		Singleton() {
			assert(!ptr_);
			ptr_ = static_cast<T *>(this);
		}
		Singleton(const Singleton<T> &) = delete;
		Singleton(const Singleton<T> &&) = delete;
		Singleton & operator=(const Singleton<T> &) = delete;
		Singleton & operator=(const Singleton<T> &&) = delete;
		~Singleton() {
			assert(ptr_);
			ptr_ = nullptr;
		}
		
		static T & getSingleton() {
			assert(ptr_);
			return *ptr_;
		}
		static T * getSingletonPtr() {
			assert(ptr_);
			return ptr_;
		}
	protected:
		static T * ptr_;
	};
}

#endif // AsteroSingleton_tpp
