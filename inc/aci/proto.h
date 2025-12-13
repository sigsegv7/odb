/*
 * Copyright (c) 2025 Ian Marco Moffett and the Osmora Team.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of Hyra nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef ACI_PROTO_H
#define ACI_PROTO_H 1

#include <stdint.h>
#include <stddef.h>
#include "aci/datatype.h"

/*
 * Valid ACI commands
 *
 * @ACI_CMD_NOP: No-operation [does nothing]
 * @ACI_CMD_STORE: Store a piece of data to a key
 * @ACI_CMD_QUERY: Query a key
 */
typedef enum {
    ACI_CMD_NOP,
    ACI_CMD_STORE,
    ACI_CMD_QUERY,
    ACI_CMD_CREATE
} aci_op_t;

/*
 * An access control interface packet
 *
 * @op: Operation code
 * @type: Operation datatype
 * @length: Length of operation
 * @data: Data associated with operation
 */
struct aci_pkt {
    aci_op_t op;
    aci_datatype_t type;
    size_t length;
    char data[];
};

/*
 * Initialize an ACI packet
 *
 * @op: Operation to associate with packet
 * @type: Datatype associated with operation
 * @length: Length of data
 * @data: Data to bundle with packet
 * @res: Result packet is written here
 */
int aci_pkt_init(
    aci_op_t op, aci_datatype_t type,
    size_t length, const void *data,
    struct aci_pkt **res
);

/*
 * Deallocate a packet from memory
 *
 * @pkt: Packet to free
 */
void aci_pkt_free(struct aci_pkt *pkt);

#endif  /* !ACI_PROTO_H */
