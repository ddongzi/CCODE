//
// Created by dong on 3/29/24.
//

#ifndef SERVER_H
#define SERVER_H

#include <stdint.h>
#include "gnutls.h"

void server_init(uint16_t port);
void server_start();

#define LOOP_CHECK(rval, cmd) \
	do {                  \
		rval = cmd;   \
	} while (rval == GNUTLS_E_AGAIN || rval == GNUTLS_E_INTERRUPTED)

#endif //SERVER_H
