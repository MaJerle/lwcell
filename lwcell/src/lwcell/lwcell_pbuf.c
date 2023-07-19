/**
 * \file            lwcell_pbuf.c
 * \brief           Packet buffer manager
 */

/*
 * Copyright (c) 2023 Tilen MAJERLE
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * This file is part of LwCELL - Lightweight GSM-AT library.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 * Version:         v0.1.1
 */
#include "lwcell/lwcell_pbuf.h"
#include "lwcell/lwcell_private.h"

/* Set size of pbuf structure */
#define SIZEOF_PBUF_STRUCT LWCELL_MEM_ALIGN(sizeof(lwcell_pbuf_t))
#define SET_NEW_LEN(v, len)                                                                                            \
    do {                                                                                                               \
        if ((v) != NULL) {                                                                                             \
            *(v) = (len);                                                                                              \
        }                                                                                                              \
    } while (0)

/**
 * \brief           Skip pbufs for desired offset
 * \param[in]       p: Source pbuf to skip
 * \param[in]       off: Offset in units of bytes to skip
 * \param[out]      new_off: New offset on new returned pbuf
 * \return          New pbuf where offset was found, `NULL` if offset too big for pbuf chain
 */
static lwcell_pbuf_p
pbuf_skip(lwcell_pbuf_p p, size_t off, size_t* new_off) {
    if (p == NULL || p->tot_len < off) { /* Check valid parameters */
        SET_NEW_LEN(new_off, 0);         /* Set output value */
        return NULL;
    }

    /* Skip pbufs until we reach offset */
    for (; p != NULL && p->len <= off; p = p->next) {
        off -= p->len; /* Decrease offset by current pbuf length */
    }

    SET_NEW_LEN(new_off, off); /* Set output value */
    return p;
}

/**
 * \brief           Allocate packet buffer for network data of specific size
 * \param[in]       len: Length of payload memory to allocate
 * \return          Pointer to allocated memory, `NULL` otherwise
 */
lwcell_pbuf_p
lwcell_pbuf_new(size_t len) {
    lwcell_pbuf_p p;

    p = lwcell_mem_malloc(SIZEOF_PBUF_STRUCT + sizeof(*p->payload) * len);
    LWCELL_DEBUGW(LWCELL_CFG_DBG_PBUF | LWCELL_DBG_TYPE_TRACE, p == NULL, "[LWCELL PBUF] Failed to allocate %u bytes\r\n",
                 (unsigned)len);
    LWCELL_DEBUGW(LWCELL_CFG_DBG_PBUF | LWCELL_DBG_TYPE_TRACE, p != NULL, "[LWCELL PBUF] Allocated %u bytes on %p\r\n",
                 (unsigned)len, (void*)p);
    if (p != NULL) {
        p->next = NULL;                                        /* No next element in chain */
        p->tot_len = len;                                      /* Set total length of pbuf chain */
        p->len = len;                                          /* Set payload length */
        p->payload = (void*)(((char*)p) + SIZEOF_PBUF_STRUCT); /* Set pointer to payload data */
        p->ref = 1;                                            /* Single reference is used on this pbuf */
    }
    return p;
}

/**
 * \brief           Free previously allocated packet buffer
 * \note            Application must not use reference to pbuf after the call to this function.
 *                  It is advised to immediately set pointer to `NULL` or to call.
 *                  Alternatively, call \ref lwcell_pbuf_free_s, which will reset the pointer
 *                  after free operation has been completed
 *                  
 * \param[in]       pbuf: Packet buffer to free
 * \return          Number of freed pbufs from head
 * \sa              lwcell_pbuf_free_s
 */
size_t
lwcell_pbuf_free(lwcell_pbuf_p pbuf) {
    lwcell_pbuf_p p, pn;
    size_t ref, cnt;

    LWCELL_ASSERT(pbuf != NULL);

    /*
     * Free all pbufs until first ->ref > 1 is reached
     * which means somebody has reference to part of pbuf and we have to keep it as is
     */
    cnt = 0;
    for (p = pbuf; p != NULL;) {
        lwcell_core_lock();
        ref = --p->ref; /* Decrease current value and save it */
        lwcell_core_unlock();
        if (ref == 0) { /* Did we reach 0 and are ready to free it? */
            LWCELL_DEBUGF(LWCELL_CFG_DBG_PBUF | LWCELL_DBG_TYPE_TRACE,
                         "[LWCELL PBUF] Deallocating %p with len/tot_len: %u/%u\r\n", (void*)p, (unsigned)p->len,
                         (unsigned)p->tot_len);
            pn = p->next;                 /* Save next entry */
            lwcell_mem_free_s((void**)&p); /* Free memory for pbuf */
            p = pn;                       /* Restore with next entry */
            ++cnt;                        /* Increase number of freed pbufs */
        } else {
            break;
        }
    }
    return cnt;
}

/**
 * \brief           Free previously allocated packet buffer in safe way.
 *                  Function accepts pointer to pointer and will set the pointer to `NULL`
 *                  after the successful allocation
 * 
 * \param[in,out]   pbuf_ptr: Pointer to pointer to packet buffer
 * \return          Number of packet buffers freed in the chain
 */
size_t
lwcell_pbuf_free_s(lwcell_pbuf_p* pbuf_ptr) {
    size_t cnt = 0;

    LWCELL_ASSERT0(pbuf_ptr != NULL);

    if (*pbuf_ptr != NULL) {
        cnt = lwcell_pbuf_free(*pbuf_ptr);
        *pbuf_ptr = NULL;
    }
    return cnt;
}

/**
 * \brief           Concatenate `2` packet buffers together to one big packet
 * \note            After `tail` pbuf has been added to `head` pbuf chain,
 *                  it must not be referenced by user anymore as it is now completely controlled by `head` pbuf.
 *                  In simple words, when user calls this function, it should not call \ref lwcell_pbuf_free function anymore,
 *                  as it might make memory undefined for `head` pbuf.
 * \param[in]       head: Head packet buffer to append new pbuf to
 * \param[in]       tail: Tail packet buffer to append to head pbuf
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 * \sa              lwcell_pbuf_cat_s
 * \sa              lwcell_pbuf_chain
 */
lwcellr_t
lwcell_pbuf_cat(lwcell_pbuf_p head, const lwcell_pbuf_p tail) {
    LWCELL_ASSERT(head != NULL);
    LWCELL_ASSERT(tail != NULL);

    /*
     * For all pbuf packets in head,
     * increase total length parameter of all next entries
     */
    for (; head->next != NULL; head = head->next) {
        head->tot_len += tail->tot_len; /* Increase total length of packet */
    }
    head->tot_len += tail->tot_len; /* Increase total length of last packet in chain */
    head->next = tail;              /* Set next packet buffer as next one */

    return lwcellOK;
}

/**
 * \brief           Concatenate `2` packet buffers together to one big packet with safe pointer management
 * \note            After `tail` pbuf has been added to `head` pbuf chain,
 *                  `tail` pointer will be set to `NULL`
 * \param[in]       head: Head packet buffer to append new pbuf to
 * \param[in]       tail: Pointer to pointer to tail packet buffer to append to head pbuf.
 *                      Pointed memory will be set to `NULL` after successful concatenation
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 * \sa              lwcell_pbuf_cat
 * \sa              lwcell_pbuf_chain
 */
lwcellr_t
lwcell_pbuf_cat_s(lwcell_pbuf_p head, lwcell_pbuf_p* tail) {
    lwcellr_t res;

    LWCELL_ASSERT(head != NULL);
    LWCELL_ASSERT(tail != NULL);

    if (*tail != NULL && (res = lwcell_pbuf_cat(head, *tail)) == lwcellOK) {
        *tail = NULL;
    }
    return res;
}

/**
 * \brief           Chain 2 pbufs together. Similar to \ref lwcell_pbuf_cat
 *                  but now new reference is done from head pbuf to tail pbuf.
 * \note            After this function call, user must call \ref lwcell_pbuf_free to remove
 *                  its reference to tail pbuf and allow control to head pbuf: `lwcell_pbuf_free(tail)`
 * \param[in]       head: Head packet buffer to append new pbuf to
 * \param[in]       tail: Tail packet buffer to append to head pbuf
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 * \sa              lwcell_pbuf_cat
 * \sa              lwcell_pbuf_cat_s
 * \sa              lwcell_pbuf_chain_s
 */
lwcellr_t
lwcell_pbuf_chain(lwcell_pbuf_p head, lwcell_pbuf_p tail) {
    lwcellr_t res;

    LWCELL_ASSERT(head != NULL);
    LWCELL_ASSERT(tail != NULL);

    /*
     * To prevent issues with multi-thread access,
     * first reference pbuf and increase counter
     */
    lwcell_pbuf_ref(tail);                                /* Reference tail pbuf by head pbuf now */
    if ((res = lwcell_pbuf_cat(head, tail)) != lwcellOK) { /* Did we concatenate them together successfully? */
        lwcell_pbuf_free(tail);                           /* Call free to decrease reference counter */
    }
    return res;
}

/**
 * \brief           Unchain first pbuf from list and return second one
 *
 * `tot_len` and `len` fields are adjusted to reflect new values and reference counter is `as is`
 *
 * \note            After unchain, user must take care of both pbufs (`head` and `new returned one`)
 * \param[in]       head: First pbuf in chain to remove from chain
 * \return          Next pbuf after `head`
 */
lwcell_pbuf_p
lwcell_pbuf_unchain(lwcell_pbuf_p head) {
    lwcell_pbuf_p r = NULL;
    if (head != NULL && head->next != NULL) { /* Check for valid pbuf */
        r = head->next;                       /* Set return value as next pbuf */

        head->next = NULL;         /* Clear next pbuf */
        head->tot_len = head->len; /* Set new length of head pbuf */
    }
    return r;
}

/**
 * \brief           Increment reference count on pbuf
 * \param[in]       pbuf: pbuf to increase reference
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 */
lwcellr_t
lwcell_pbuf_ref(lwcell_pbuf_p pbuf) {
    LWCELL_ASSERT(pbuf != NULL);

    ++pbuf->ref; /* Increase reference count for pbuf */
    return lwcellOK;
}

/**
 * \brief           Copy user data to chain of pbufs
 * \param[in]       pbuf: First pbuf in chain to start copying to
 * \param[in]       data: Input data to copy to pbuf memory
 * \param[in]       len: Length of input data to copy
 * \param[in]       offset: Start offset in pbuf where to start copying
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 */
lwcellr_t
lwcell_pbuf_take(lwcell_pbuf_p pbuf, const void* data, size_t len, size_t offset) {
    const uint8_t* d = data;
    size_t copy_len;

    LWCELL_ASSERT(pbuf != NULL);
    LWCELL_ASSERT(data != NULL);
    LWCELL_ASSERT(len > 0);
    LWCELL_ASSERT(pbuf->tot_len >= len);

    /* Skip if necessary and check if we are in valid range */
    if (offset > 0) {
        pbuf = pbuf_skip(pbuf, offset, &offset); /* Offset and check for new length */
        if (pbuf == NULL) {
            return lwcellERR;
        }
    }

    if (pbuf->tot_len < (len + offset)) {
        return lwcellERRPAR;
    }

    /* First only copy in case we have some offset from first pbuf */
    if (offset > 0) {
        copy_len = LWCELL_MIN(pbuf->len - offset, len);     /* Get length to copy to current pbuf */
        LWCELL_MEMCPY(pbuf->payload + offset, d, copy_len); /* Copy to memory with offset */
        len -= copy_len;                                   /* Decrease remaining bytes to copy */
        d += copy_len;                                     /* Increase data pointer */
        pbuf = pbuf->next;                                 /* Go to next pbuf */
    }

    /* Copy user memory to sequence of pbufs */
    for (; len; pbuf = pbuf->next) {
        copy_len = LWCELL_MIN(len, pbuf->len);     /* Get copy length */
        LWCELL_MEMCPY(pbuf->payload, d, copy_len); /* Copy memory to pbuf payload */
        len -= copy_len;                          /* Decrease number of remaining bytes to send */
        d += copy_len;                            /* Increase data pointer */
    }
    return lwcellOK;
}

/**
 * \brief           Copy memory from pbuf to user linear memory
 * \param[in]       pbuf: Pbuf to copy from
 * \param[out]      data: User linear memory to copy to
 * \param[in]       len: Length of data in units of bytes
 * \param[in]       offset: Possible start offset in pbuf
 * \return          Number of bytes copied
 */
size_t
lwcell_pbuf_copy(lwcell_pbuf_p pbuf, void* data, size_t len, size_t offset) {
    size_t tot, tc;
    uint8_t* d = data;

    if (pbuf == NULL || data == NULL || len == 0 || pbuf->tot_len < offset) {
        return 0;
    }

    /*
     * In case user wants offset,
     * skip to necessary pbuf
     */
    if (offset > 0) {
        pbuf = pbuf_skip(pbuf, offset, &offset); /* Skip offset if necessary */
        if (pbuf == NULL) {
            return 0;
        }
    }

    /*
     * Copy data from pbufs to memory
     * with checking for initial offset (only one can have offset)
     */
    tot = 0;
    for (; pbuf != NULL && len; pbuf = pbuf->next) {
        tc = LWCELL_MIN(pbuf->len - offset, len);     /* Get length of data to copy */
        LWCELL_MEMCPY(d, pbuf->payload + offset, tc); /* Copy data from pbuf */
        d += tc;
        len -= tc;
        tot += tc;
        offset = 0; /* No more offset in this case */
    }
    return tot;
}

/**
 * \brief           Get value from pbuf at specific position
 * \param[in]       pbuf: Pbuf used to get data from
 * \param[in]       pos: Position at which to get element
 * \param[out]      el: Output variable to save element value at desired position
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwcell_pbuf_get_at(const lwcell_pbuf_p pbuf, size_t pos, uint8_t* el) {
    lwcell_pbuf_p p;

    if (pbuf != NULL) {
        p = pbuf_skip(pbuf, pos, &pos); /* Skip pbufs to desired position and get new offset from new pbuf */
        if (p != NULL) {                /* Do we have new pbuf? */
            *el = p->payload[pos];      /* Return memory at desired new offset from latest pbuf */
            return 1;
        }
    }
    return 0; /* Invalid character */
}

/**
 * \brief           Find desired needle in a haystack
 * \param[in]       pbuf: Pbuf used as haystack
 * \param[in]       needle: Data memory used as needle
 * \param[in]       len: Length of needle memory
 * \param[in]       off: Starting offset in pbuf memory
 * \return          `LWCELL_SIZET_MAX` if no match or position where in pbuf we have a match
 * \sa              lwcell_pbuf_strfind
 */
size_t
lwcell_pbuf_memfind(const lwcell_pbuf_p pbuf, const void* needle, size_t len, size_t off) {
    if (pbuf != NULL && needle != NULL && pbuf->tot_len >= (len + off)) { /* Check if valid entries */
        /*
         * Try entire buffer element by element
         * and in case we have a match, report it
         */
        for (size_t i = off; i <= pbuf->tot_len - len; ++i) {
            if (!lwcell_pbuf_memcmp(pbuf, needle, len, i)) { /* Check if identical */
                return i;                                   /* We have a match! */
            }
        }
    }
    return LWCELL_SIZET_MAX; /* Return maximal value of size_t variable to indicate error */
}

/**
 * \brief           Find desired needle (str) in a haystack (pbuf)
 * \param[in]       pbuf: Pbuf used as haystack
 * \param[in]       str: String to search for in pbuf
 * \param[in]       off: Starting offset in pbuf memory
 * \return          `LWCELL_SIZET_MAX` if no match or position where in pbuf we have a match
 * \sa              lwcell_pbuf_memfind
 */
size_t
lwcell_pbuf_strfind(const lwcell_pbuf_p pbuf, const char* str, size_t off) {
    return lwcell_pbuf_memfind(pbuf, str, strlen(str), off);
}

/**
 * \brief           Compare pbuf memory with memory from data
 * \note            Compare is done on entire pbuf chain
 * \param[in]       pbuf: Pbuf used to compare with data memory
 * \param[in]       data: Actual data to compare with
 * \param[in]       len: Length of input data in units of bytes
 * \param[in]       offset: Start offset to use when comparing data
 * \return          `0` if equal, `LWCELL_SIZET_MAX` if memory/offset too big or anything between if not equal
 * \sa              lwcell_pbuf_strcmp
 */
size_t
lwcell_pbuf_memcmp(const lwcell_pbuf_p pbuf, const void* data, size_t len, size_t offset) {
    lwcell_pbuf_p p;
    uint8_t el;
    const uint8_t* d = data;

    if (pbuf == NULL || data == NULL || len == 0 /* Input parameters check */
        || pbuf->tot_len < (offset + len)) {     /* Check of valid ranges */
        return LWCELL_SIZET_MAX;                  /* Invalid check here */
    }

    /*
     * Find start pbuf to have more optimized search at the end
     * Since we had a check on beginning, we must pass this for loop without any problems
     */
    for (p = pbuf; p != NULL && p->len <= offset; p = p->next) {
        offset -= p->len; /* Decrease offset by length of pbuf */
    }

    /*
     * We have known starting pbuf.
     * Now it is time to check byte by byte from pbuf and memory
     *
     * Use byte by byte read function to inspect bytes separatelly
     */
    for (size_t i = 0; i < len; ++i) {
        if (!lwcell_pbuf_get_at(p, offset + i, &el) || el != d[i]) { /* Get value from pbuf at specific offset */
            return offset + 1;                                      /* Return value from offset where it failed */
        }
    }
    return 0; /* Memory matches at this point */
}

/**
 * \brief           Compare pbuf memory with input string
 * \note            Compare is done on entire pbuf chain
 * \param[in]       pbuf: Pbuf used to compare with data memory
 * \param[in]       str: String to be compared with pbuf
 * \param[in]       offset: Start memory offset in pbuf
 * \return          `0` if equal, `LWCELL_SIZET_MAX` if memory/offset too big or anything between if not equal
 * \sa              lwcell_pbuf_memcmp
 */
size_t
lwcell_pbuf_strcmp(const lwcell_pbuf_p pbuf, const char* str, size_t offset) {
    return lwcell_pbuf_memcmp(pbuf, str, strlen(str), offset);
}

/**
 * \brief           Get linear offset address for pbuf from specific offset
 * \note            Since pbuf memory can be fragmented in chain,
 *                  you may need to call function multiple times to get memory for entire pbuf chain
 * \param[in]       pbuf: Pbuf to get linear address
 * \param[in]       offset: Start offset from where to start
 * \param[out]      new_len: Length of memory returned by function
 * \return          Pointer to memory on success, `NULL` otherwise
 */
void*
lwcell_pbuf_get_linear_addr(const lwcell_pbuf_p pbuf, size_t offset, size_t* new_len) {
    lwcell_pbuf_p p = pbuf;

    if (pbuf == NULL || pbuf->tot_len < offset) { /* Check input parameters */
        SET_NEW_LEN(new_len, 0);
        return NULL;
    }
    if (offset > 0) {                         /* Is there any offset? */
        p = pbuf_skip(pbuf, offset, &offset); /* Skip pbuf to desired length */
        if (p == NULL) {
            SET_NEW_LEN(new_len, 0);
            return NULL;
        }
    }

    SET_NEW_LEN(new_len, p->len - offset);
    return &p->payload[offset]; /* Return memory at desired offset */
}

/**
 * \brief           Get data pointer from packet buffer
 * \param[in]       pbuf: Packet buffer
 * \return          Pointer to data buffer on success, `NULL` otherwise
 */
void*
lwcell_pbuf_data(const lwcell_pbuf_p pbuf) {
    return pbuf != NULL ? pbuf->payload : NULL;
}

/**
 * \brief           Get length of packet buffer
 * \param[in]       pbuf: Packet buffer to get length for
 * \param[in]       tot: Set to `1` to return total packet chain length or `0` to get only first packet length
 * \return          Length of data in units of bytes
 */
size_t
lwcell_pbuf_length(const lwcell_pbuf_p pbuf, uint8_t tot) {
    return pbuf != NULL ? (tot ? pbuf->tot_len : pbuf->len) : 0;
}

/**
 * \brief           Set IP address and port number for received data
 * \param[in]       pbuf: Packet buffer
 * \param[in]       ip: IP to assing to packet buffer
 * \param[in]       port: Port number to assign to packet buffer
 */
void
lwcell_pbuf_set_ip(lwcell_pbuf_p pbuf, const lwcell_ip_t* ip, lwcell_port_t port) {
    if (pbuf != NULL && ip != NULL) {
        LWCELL_MEMCPY(&pbuf->ip, ip, sizeof(*ip));
        pbuf->port = port;
    }
}

/**
 * \brief           Advance pbuf payload pointer by number of len bytes.
 *                  It can only advance single pbuf in a chain
 *
 * \note            When other pbufs are referencing current one,
 *                  they are not adjusted in length and total length
 *
 * \param[in]       pbuf: Pbuf to advance
 * \param[in]       len: Number of bytes to advance.
 *                      when negative is used, buffer size is increased only if it was decreased before
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwcell_pbuf_advance(lwcell_pbuf_p pbuf, int len) {
    uint8_t process = 0;
    if (pbuf == NULL || len == 0) {
        return 0;
    }
    if (len > 0) {                      /* When we want to decrease size */
        if ((size_t)len <= pbuf->len) { /* Is there space to decrease? */
            process = 1;
        }
    } else {
        /* Is current payload + new len still higher than pbuf structure? */
        if (((uint8_t*)pbuf + SIZEOF_PBUF_STRUCT) < (pbuf->payload + len)) {
            process = 1;
        }
    }
    if (process) {
        pbuf->payload += len; /* Increase payload pointer */
        pbuf->tot_len -= len; /* Decrease length of pbuf chain */
        pbuf->len -= len;     /* Decrease length of current pbuf */
    }
    return process;
}

/**
 * \brief           Skip a list of pbufs for desired offset
 * \note            Reference is not changed after return and user must not free the memory of new pbuf directly
 * \param[in]       pbuf: Start of pbuf chain
 * \param[in]       offset: Offset in units of bytes to skip
 * \param[out]      new_offset: Pointer to output variable to save new offset in returned pbuf
 * \return          New pbuf on success, `NULL` otherwise
 */
lwcell_pbuf_p
lwcell_pbuf_skip(lwcell_pbuf_p pbuf, size_t offset, size_t* new_offset) {
    return pbuf_skip(pbuf, offset, new_offset); /* Skip pbufs with internal function */
}

/**
 * \brief           Dump and debug pbuf chain
 * \param[in]       p: Head pbuf to dump
 * \param[in]       seq: Set to `1` to dump all `pbufs` in linked list or `0` to dump first one only
 */
void
lwcell_pbuf_dump(lwcell_pbuf_p p, uint8_t seq) {
    if (p != NULL) {
        LWCELL_DEBUGF(LWCELL_CFG_DBG_PBUF | LWCELL_DBG_TYPE_TRACE, "[LWCELL PBUF] Dump start: %p\r\n", (void*)p);
        for (; p != NULL; p = p->next) {
            LWCELL_DEBUGF(LWCELL_CFG_DBG_PBUF | LWCELL_DBG_TYPE_TRACE,
                         "[LWCELL PBUF] Dump %p; ref: %u; len: %u; tot_len: %u, next: %p\r\n", (void*)p,
                         (unsigned)p->ref, (unsigned)p->len, (unsigned)p->tot_len, (void*)p->next);
            if (!seq) {
                break;
            }
        }
        LWCELL_DEBUGF(LWCELL_CFG_DBG_PBUF | LWCELL_DBG_TYPE_TRACE, "[LWCELL PBUF] Dump end\r\n");
    }
}
