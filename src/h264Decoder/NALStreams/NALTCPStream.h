#ifndef NALTCPSTREAM_H_
#define NALTCPSTREAM_H_

#include "NALNetworkStream.h"

namespace H264 {

class NALTCPStream: public H264::NALNetworkStream {
public:
	NALTCPStream();
	virtual ~NALTCPStream();
};

}

#endif /* NALTCPSTREAM_H_ */
