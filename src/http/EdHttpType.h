/*
 * EdHttpType.h
 *
 *  Created on: Sep 15, 2014
 *      Author: netmind
 */

#ifndef EDHTTPTYPE_H_
#define EDHTTPTYPE_H_
#include "../config.h"


namespace edft {

typedef struct {
	int len;
	void* buf;
} packet_buf_t ;

} // namespace edft
#endif /* EDHTTPTYPE_H_ */
