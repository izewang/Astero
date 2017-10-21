//
//  AsteroContainers.tpp
//  Astero
//
//  Created by Yuzhe Wang on 8/9/17.
//  Copyright © 2017 Yuzhe Wang. All rights reserved.
//

#ifndef AsteroContainers_tpp
#define AsteroContainers_tpp

#include "AsteroAllocator.tpp"

// containers with custom allocator
#include <deque>
#include <vector>
#include <list>
#include <set>
#include <map>

namespace Astero {
	template <typename T, typename A = STLAllocator<T, GeneralAllocPolicy> >
	struct deque
	{
		typedef typename std::deque<T, A> type;
		typedef typename std::deque<T, A>::iterator iterator;
		typedef typename std::deque<T, A>::const_iterator const_iterator;
	};
	
	template <typename T, typename A = STLAllocator<T, GeneralAllocPolicy> >
	struct vector
	{
		typedef typename std::vector<T, A> type;
		typedef typename std::vector<T, A>::iterator iterator;
		typedef typename std::vector<T, A>::const_iterator const_iterator;
	};
	
	template <typename T, typename A = STLAllocator<T, GeneralAllocPolicy> >
	struct list
	{
		typedef typename std::list<T, A> type;
		typedef typename std::list<T, A>::iterator iterator;
		typedef typename std::list<T, A>::const_iterator const_iterator;
	};
	
	template <typename T, typename P = std::less<T>, typename A = STLAllocator<T, GeneralAllocPolicy> >
	struct set
	{
		typedef typename std::set<T, P, A> type;
		typedef typename std::set<T, P, A>::iterator iterator;
		typedef typename std::set<T, P, A>::const_iterator const_iterator;
	};
	
	template <typename K, typename V, typename P = std::less<K>, typename A = STLAllocator<std::pair<const K, V>, GeneralAllocPolicy> >
	struct map
	{
		typedef typename std::map<K, V, P, A> type;
		typedef typename std::map<K, V, P, A>::iterator iterator;
		typedef typename std::map<K, V, P, A>::const_iterator const_iterator;
	};
	
	template <typename K, typename V, typename P = std::less<K>, typename A = STLAllocator<std::pair<const K, V>, GeneralAllocPolicy> >
	struct multimap
	{
		typedef typename std::multimap<K, V, P, A> type;
		typedef typename std::multimap<K, V, P, A>::iterator iterator;
		typedef typename std::multimap<K, V, P, A>::const_iterator const_iterator;
	};
} // namespace Astero

#endif // AsteroContainers_tpp
