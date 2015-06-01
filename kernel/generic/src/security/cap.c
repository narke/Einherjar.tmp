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

/** @addtogroup generic
 * @{
 */

/**
 * @file cap.c
 * @brief Capabilities control.
 *
 * @see cap.h
 */

#include <security/cap.h>
#include <proc/task.h>
#include <synch/spinlock.h>
#include <arch.h>
#include <errno.h>

/** Set capabilities.
 *
 * @param task Task whose capabilities are to be changed.
 * @param caps New set of capabilities.
 *
 */
void cap_set(task_t *task, cap_t caps)
{
	irq_spinlock_lock(&task->lock, true);
	task->capabilities = caps;
	irq_spinlock_unlock(&task->lock, true);
}

/** Get capabilities.
 *
 * @param task Task whose capabilities are to be returned.
 *
 * @return Task's capabilities.
 *
 */
cap_t cap_get(task_t *task)
{
	irq_spinlock_lock(&task->lock, true);
	cap_t caps = task->capabilities;
	irq_spinlock_unlock(&task->lock, true);
	
	return caps;
}

/** @}
 */
