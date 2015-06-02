/*
 * Copyright (c) 2006 Jakub Jermar
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

/** @addtogroup genericddi
 * @{
 */

/**
 * @file
 * @brief Device Driver Interface functions.
 *
 * This file contains functions that comprise the Device Driver Interface.
 * These are the functions for mapping physical memory and enabling I/O
 * space to tasks.
 */

#include <ddi/ddi.h>
#include <proc/task.h>
#include <mm/frame.h>
#include <mm/as.h>
#include <mm/page.h>
#include <synch/mutex.h>
#include <adt/btree.h>
#include <arch.h>
#include <align.h>
#include <errno.h>
#include <trace.h>
#include <bitops.h>

/** This lock protects the parea_btree. */
static mutex_t parea_lock;

/** B+tree with enabled physical memory areas. */
static btree_t parea_btree;

/** Initialize DDI.
 *
 */
void ddi_init(void)
{
	btree_create(&parea_btree);
	mutex_initialize(&parea_lock, MUTEX_PASSIVE);
}

/** Enable piece of physical memory for mapping by physmem_map().
 *
 * @param parea Pointer to physical area structure.
 *
 */
void ddi_parea_register(parea_t *parea)
{
	mutex_lock(&parea_lock);
	
	/*
	 * We don't check for overlaps here as the kernel is pretty sane.
	 */
	btree_insert(&parea_btree, (btree_key_t) parea->pbase, parea, NULL);
	
	mutex_unlock(&parea_lock);
}


/** @}
 */
