/*
 * Copyright (c) 2008 Open Kernel Labs, Inc. (Copyright Holder).
 * All rights reserved.
 *
 * 1. Redistribution and use of OKL4 (Software) in source and binary
 * forms, with or without modification, are permitted provided that the
 * following conditions are met:
 *
 *     (a) Redistributions of source code must retain this clause 1
 *         (including paragraphs (a), (b) and (c)), clause 2 and clause 3
 *         (Licence Terms) and the above copyright notice.
 *
 *     (b) Redistributions in binary form must reproduce the above
 *         copyright notice and the Licence Terms in the documentation and/or
 *         other materials provided with the distribution.
 *
 *     (c) Redistributions in any form must be accompanied by information on
 *         how to obtain complete source code for:
 *        (i) the Software; and
 *        (ii) all accompanying software that uses (or is intended to
 *        use) the Software whether directly or indirectly.  Such source
 *        code must:
 *        (iii) either be included in the distribution or be available
 *        for no more than the cost of distribution plus a nominal fee;
 *        and
 *        (iv) be licensed by each relevant holder of copyright under
 *        either the Licence Terms (with an appropriate copyright notice)
 *        or the terms of a licence which is approved by the Open Source
 *        Initative.  For an executable file, "complete source code"
 *        means the source code for all modules it contains and includes
 *        associated build and other files reasonably required to produce
 *        the executable.
 *
 * 2. THIS SOFTWARE IS PROVIDED ``AS IS'' AND, TO THE EXTENT PERMITTED BY
 * LAW, ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE, OR NON-INFRINGEMENT, ARE DISCLAIMED.  WHERE ANY WARRANTY IS
 * IMPLIED AND IS PREVENTED BY LAW FROM BEING DISCLAIMED THEN TO THE
 * EXTENT PERMISSIBLE BY LAW: (A) THE WARRANTY IS READ DOWN IN FAVOUR OF
 * THE COPYRIGHT HOLDER (AND, IN THE CASE OF A PARTICIPANT, THAT
 * PARTICIPANT) AND (B) ANY LIMITATIONS PERMITTED BY LAW (INCLUDING AS TO
 * THE EXTENT OF THE WARRANTY AND THE REMEDIES AVAILABLE IN THE EVENT OF
 * BREACH) ARE DEEMED PART OF THIS LICENCE IN A FORM MOST FAVOURABLE TO
 * THE COPYRIGHT HOLDER (AND, IN THE CASE OF A PARTICIPANT, THAT
 * PARTICIPANT). IN THE LICENCE TERMS, "PARTICIPANT" INCLUDES EVERY
 * PERSON WHO HAS CONTRIBUTED TO THE SOFTWARE OR WHO HAS BEEN INVOLVED IN
 * THE DISTRIBUTION OR DISSEMINATION OF THE SOFTWARE.
 *
 * 3. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR ANY OTHER PARTICIPANT BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <okl4/types.h>
#include <okl4/irqset.h>
#include <okl4/errno.h>

#include <l4/interrupt.h>
#include <l4/utcb.h>
#include <l4/ipc.h>

static okl4_word_t
wait_for_interrupt(okl4_irqset_t *set)
{
    okl4_word_t success;

    /* Set our mask to only care about interrupts. */
    L4_Set_NotifyMask(1 << set->notify_bits);

    /* If we haven't acknowledged our last IRQ, we need to do that now. */
    if (set->last_irq != ~0UL) {
        L4_LoadMR(0, set->last_irq);
        set->last_irq = ~0UL;
        success = L4_AcknowledgeWaitInterrupt(0, 0);
        assert(success);
    } else {
        /* Wait for the notify bit. */
        L4_MsgTag_t tag;
        okl4_word_t bits;
        tag = L4_WaitNotify(&bits);
        assert(L4_IpcSucceeded(tag));
        assert(bits == 1 << set->notify_bits);
    }

    /* Return the interrupt that fired. */
    return __L4_TCR_PlatformReserved(0);
}

int
okl4_irqset_wait(okl4_irqset_t *set, okl4_word_t *result_irq)
{
    okl4_word_t irq;

    /* Ensure we are registered to at least one interrupt. */
    assert(set->num_interrupts > 0);

    /*
     * Wait for an interrupt.
     *
     * It may be that the kernel delivers us an IRQ number of (-1). This occurs
     * when an interrupt fires just before we deregister it, leaving us with
     * a stale notification bit. In this case, we can't do much more than try
     * again.
     */
    do {
        irq = wait_for_interrupt(set);
    } while (irq == ~0UL);

    set->last_irq = irq;
    *result_irq = irq;
    return OKL4_OK;
}

