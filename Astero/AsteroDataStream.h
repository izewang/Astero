//
//  DataStream.h
//  Astero
//
//  Created by Yuzhe Wang on 8/26/17.
//  Copyright © 2017 Yuzhe Wang. All rights reserved.
//

#ifndef AsteroDataStream_h
#define AsteroDataStream_h

namespace Astero {
	class DataStream {
	public:
		DataStream() {}
		DataStream(const DataStream & data_stream) {}
	};
	typedef std::shared_ptr<DataStream> DataStreamPtr;
	
	class MemoryDataStream : public DataStream {
	public:
		MemoryDataStream() {}
		MemoryDataStream(const std::string & name, DataStreamPtr & ptr) : DataStream() {}
	};
}


#endif // AsteroDataStream_h
