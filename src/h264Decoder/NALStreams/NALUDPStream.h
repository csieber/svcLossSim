#ifndef NALUDPSTREAM_H_
#define NALUDPSTREAM_H_

#include "NALNetworkStream.h"

namespace H264 {

class NALUDPStream: public H264::NALNetworkStream {
public:
	NALUDPStream();
	virtual ~NALUDPStream();
};

}

#endif /* NALUDPSTREAM_H_ */
