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

/** @addtogroup sync
 * @{
 */

/**
 * @file
 * @brief	Kernel backend for futexes.
 */

#include <synch/futex.h>
#include <synch/mutex.h>
#include <synch/spinlock.h>
#include <mm/frame.h>
#include <mm/page.h>
#include <mm/slab.h>
#include <proc/thread.h>
#include <proc/task.h>
#include <genarch/mm/page_pt.h>
#include <genarch/mm/page_ht.h>
#include <adt/hash_table.h>
#include <adt/list.h>
#include <arch.h>
#include <align.h>
#include <panic.h>
#include <errno.h>
#include <print.h>

#define FUTEX_HT_SIZE	1024	/* keep it a power of 2 */

static size_t futex_ht_hash(sysarg_t *key);
static bool futex_ht_compare(sysarg_t *key, size_t keys, link_t *item);
static void futex_ht_remove_callback(link_t *item);

/**
 * Mutex protecting global futex hash table.
 * It is also used to serialize access to all futex_t structures.
 * Must be acquired before the task futex B+tree lock.
 */
static mutex_t futex_ht_lock;

/** Futex hash table. */
static hash_table_t futex_ht;

/** Futex hash table operations. */
static hash_table_operations_t futex_ht_ops = {
	.hash = futex_ht_hash,
	.compare = futex_ht_compare,
	.remove_callback = futex_ht_remove_callback
};

/** Initialize futex subsystem. */
void futex_init(void)
{
	mutex_initialize(&futex_ht_lock, MUTEX_PASSIVE);
	hash_table_create(&futex_ht, FUTEX_HT_SIZE, 1, &futex_ht_ops);
}

/** Compute hash index into futex hash table.
 *
 * @param key		Address where the key (i.e. physical address of futex
 *			counter) is stored.
 *
 * @return		Index into futex hash table.
 */
size_t futex_ht_hash(sysarg_t *key)
{
	return (*key & (FUTEX_HT_SIZE - 1));
}

/** Compare futex hash table item with a key.
 *
 * @param key		Address where the key (i.e. physical address of futex
 *			counter) is stored.
 *
 * @return		True if the item matches the key. False otherwise.
 */
bool futex_ht_compare(sysarg_t *key, size_t keys, link_t *item)
{
	futex_t *futex;

	ASSERT(keys == 1);

	futex = hash_table_get_instance(item, futex_t, ht_link);
	return *key == futex->paddr;
}

/** Callback for removal items from futex hash table.
 *
 * @param item		Item removed from the hash table.
 */
void futex_ht_remove_callback(link_t *item)
{
	futex_t *futex;

	futex = hash_table_get_instance(item, futex_t, ht_link);
	free(futex);
}

/** Remove references from futexes known to the current task. */
void futex_cleanup(void)
{
	mutex_lock(&futex_ht_lock);
	mutex_lock(&TASK->futexes_lock);

	list_foreach(TASK->futexes.leaf_list, leaf_link, btree_node_t, node) {
		unsigned int i;
		
		for (i = 0; i < node->keys; i++) {
			futex_t *ftx;
			uintptr_t paddr = node->key[i];
			
			ftx = (futex_t *) node->value[i];
			if (--ftx->refcount == 0)
				hash_table_remove(&futex_ht, &paddr, 1);
		}
	}
	
	mutex_unlock(&TASK->futexes_lock);
	mutex_unlock(&futex_ht_lock);
}

/** @}
 */
