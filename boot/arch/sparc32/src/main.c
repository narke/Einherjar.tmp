/*
 * Copyright (c) 2013 Jakub Klama
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - The name of the author may not be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/** @addtogroup sparc32boot
 * @{
 */
/** @file
 * @brief Bootstrap.
 */

#include <arch/asm.h>
#include <arch/common.h>
#include <arch/arch.h>
#include <arch/ambapp.h>
#include <arch/mm.h>
#include <arch/main.h>
#include <arch/_components.h>
#include <halt.h>
#include <printf.h>
#include <memstr.h>
#include <version.h>
#include <macros.h>
#include <align.h>
#include <str.h>
#include <errno.h>
#include <inflate.h>

#define TOP2ADDR(top)  (((void *) PA2KA(BOOT_OFFSET)) + (top))

static bootinfo_t bootinfo;

void bootstrap(void)
{
	/* Initialize AMBA P&P device list */
	ambapp_scan();
	
	/* Look for UART */
	amba_device_t *uart = ambapp_lookup_first(GAISLER, GAISLER_APBUART);
	amba_uart_base = uart->bars[0].start;
	bootinfo.uart_base = amba_uart_base;
	bootinfo.uart_irq = uart->irq;
	
	/* Look for IRQMP */
	amba_device_t *irqmp = ambapp_lookup_first(GAISLER, GAISLER_IRQMP);
	bootinfo.intc_base = irqmp->bars[0].start;
	
	/* Look for timer */
	amba_device_t *timer = ambapp_lookup_first(GAISLER, GAISLER_GPTIMER);
	bootinfo.timer_base = timer->bars[0].start;
	bootinfo.timer_irq = timer->irq;
	
	/* Look for memory controller and obtain memory size */
	if (!ambapp_fake()) {
		amba_device_t *mctrl = ambapp_lookup_first(ESA, ESA_MCTRL);
		volatile mctrl_mcfg2_t *mcfg2 = (volatile mctrl_mcfg2_t *)
		    (mctrl->bars[0].start + 0x4);
		bootinfo.memsize = (1 << (13 + mcfg2->bank_size));
	} else
		bootinfo.memsize = 64 * 1024 * 1024;
	
	/* Standard output is now initialized */
	version_print();
	
	for (size_t i = 0; i < COMPONENTS; i++) {
		printf(" %p|%p: %s image (%u/%u bytes)\n", components[i].start,
		    components[i].start, components[i].name, components[i].inflated,
		    components[i].size);
	}
	
	ambapp_print_devices();
	
	printf("Memory size: %u MB\n", bootinfo.memsize >> 20);
	
	mmu_init();
	
	void *dest[COMPONENTS];
	size_t top = 0;
	size_t cnt = 0;
	bootinfo.cnt = 0;
	for (size_t i = 0; i < min(COMPONENTS, TASKMAP_MAX_RECORDS); i++) {
		top = ALIGN_UP(top, PAGE_SIZE);
		
		if (i > 0) {
			bootinfo.tasks[bootinfo.cnt].addr = TOP2ADDR(top);
			bootinfo.tasks[bootinfo.cnt].size = components[i].inflated;
			
			str_cpy(bootinfo.tasks[bootinfo.cnt].name,
			    BOOTINFO_TASK_NAME_BUFLEN, components[i].name);
			
			bootinfo.cnt++;
		}
		
		dest[i] = TOP2ADDR(top);
		
		top += components[i].inflated;
		cnt++;
	}
	
	printf("\nInflating components ... ");
	
	for (size_t i = cnt; i > 0; i--) {
		void *tail = components[i - 1].start + components[i - 1].size;
		if (tail >= dest[i - 1]) {
			printf("\n%s: Image too large to fit (%p >= %p), halting.\n",
			    components[i].name, tail, dest[i - 1]);
			halt();
		}
		
		printf("%s ", components[i - 1].name);
		
		int err = inflate(components[i - 1].start, components[i - 1].size,
		    dest[i - 1], components[i - 1].inflated);
		if (err != EOK) {
			printf("\n%s: Inflating error %d\n", components[i - 1].name, err);
			halt();
		}
	}
	
	printf("Booting the kernel ... \n");
	jump_to_kernel((void *) PA2KA(BOOT_OFFSET), &bootinfo);
}

/** @}
 */
