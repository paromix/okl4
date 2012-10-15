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

/*
 * ARM Interrupt Handling.
 */

#include <nano.h>
#include <arch/platform.h>
#include <serial.h>

/* PXA255 Interrupt controller base address. Patched by elfweaver. */
word_t interrupt_mem0 = 1;

#define PXA255_ICIP     0x0  /* Interrupt control IRQ pending. */
#define PXA255_ICMR     0x4  /* Interrupt control mask register. */
#define PXA255_ICLR     0x8  /* Interrupt control level regsiter. */
#define PXA255_ICFP     0xc  /* Interrupt control FIQ pending. */
#define PXA255_ICPR     0x10  /* Interrupt control global pending. */

/*
 * Read a hardware register.
 */
static inline uint32_t
read_reg(uint32_t addr)
{
    return *(volatile uint32_t *)addr;
}

/*
 * Write to a hardware register.
 */
static inline void
write_reg(uint32_t addr, uint32_t val)
{
    *(volatile uint32_t *)addr = val;
}

/*
 * Get the interrupt number
 */
int
plat_decode_interrupt(void)
{
    int result;
    int attempts = 0x1000;

    /*
     * The PXA255 is quite fun. It will trigger an interrupt before actually
     * setting the status bit to true. By the time we get here, it is still not
     * ready.
     *
     * We loop for a bit waiting for it to come in. If, after waiting for a
     * while, nothing appears to be coming, return an error. Emperical testing
     * shows that this can exceed 256 loops (but still have the status bit come
     * in).
     */
    do {
        result = 31 - clz(read_reg(interrupt_mem0 + PXA255_ICPR) &
                read_reg(interrupt_mem0 + PXA255_ICMR));
    } while (result == -1 && attempts-- > 0);
    return result;
}

/*
 * Unmask the given interrupts.
 */
void
plat_unmask_interrupt(int irq)
{
    write_reg(interrupt_mem0 + PXA255_ICMR,
              read_reg(interrupt_mem0 + PXA255_ICMR) | (1UL << irq));
}

/*
 * Mask the given interrupts.
 */
void
plat_mask_interrupt(int irq)
{
    write_reg(interrupt_mem0 + PXA255_ICMR,
              read_reg(interrupt_mem0 + PXA255_ICMR) & ~(1UL << irq));
}
