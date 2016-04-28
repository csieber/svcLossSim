#ifndef NALNETWORKSTREAM_H_
#define NALNETWORKSTREAM_H_

#include "NALStream.h"

namespace H264 {

class NALNetworkStream: public H264::NALStream {

public:
	NALNetworkStream();
	virtual ~NALNetworkStream();

	virtual bool listen(int32_t iPort) = 0;
};

}

#endif /* NALNETWORKSTREAM_H_ */
