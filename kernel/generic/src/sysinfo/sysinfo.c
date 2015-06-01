/*
 * Copyright (c) 2006 Jakub Vana
 * Copyright (c) 2012 Martin Decky
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
/** @file
 */

#include <sysinfo/sysinfo.h>
#include <mm/slab.h>
#include <print.h>
#include <synch/mutex.h>
#include <arch/asm.h>
#include <errno.h>
#include <macros.h>

/** Maximal sysinfo path length */
#define SYSINFO_MAX_PATH  2048

bool fb_exported = false;

/** Global sysinfo tree root item */
static sysinfo_item_t *global_root = NULL;

/** Sysinfo SLAB cache */
static slab_cache_t *sysinfo_item_slab;

/** Sysinfo lock */
static mutex_t sysinfo_lock;

/** Sysinfo item constructor
 *
 */
NO_TRACE static int sysinfo_item_constructor(void *obj, unsigned int kmflag)
{
	sysinfo_item_t *item = (sysinfo_item_t *) obj;
	
	item->name = NULL;
	item->val_type = SYSINFO_VAL_UNDEFINED;
	item->subtree_type = SYSINFO_SUBTREE_NONE;
	item->subtree.table = NULL;
	item->next = NULL;
	
	return 0;
}

/** Sysinfo item destructor
 *
 * Note that the return value is not perfectly correct
 * since more space might get actually freed thanks
 * to the disposal of item->name
 *
 */
NO_TRACE static size_t sysinfo_item_destructor(void *obj)
{
	sysinfo_item_t *item = (sysinfo_item_t *) obj;
	
	if (item->name != NULL)
		free(item->name);
	
	return 0;
}

/** Initialize sysinfo subsystem
 *
 * Create SLAB cache for sysinfo items.
 *
 */
void sysinfo_init(void)
{
	sysinfo_item_slab = slab_cache_create("sysinfo_item_t",
	    sizeof(sysinfo_item_t), 0, sysinfo_item_constructor,
	    sysinfo_item_destructor, SLAB_CACHE_MAGDEFERRED);
	
	mutex_initialize(&sysinfo_lock, MUTEX_ACTIVE);
}

/** Recursively create items in sysinfo tree
 *
 * Should be called with sysinfo_lock held.
 *
 * @param name     Current sysinfo path suffix.
 * @param psubtree Pointer to an already existing (sub)tree root
 *                 item or where to store a new tree root item.
 *
 * @return Existing or newly allocated sysinfo item or NULL
 *         if the current tree configuration does not allow to
 *         create a new item.
 *
 */
NO_TRACE static sysinfo_item_t *sysinfo_create_path(const char *name,
    sysinfo_item_t **psubtree)
{
	ASSERT(psubtree != NULL);
	
	if (*psubtree == NULL) {
		/* No parent */
		
		size_t i = 0;
		
		/* Find the first delimiter in name */
		while ((name[i] != 0) && (name[i] != '.'))
			i++;
		
		*psubtree =
		    (sysinfo_item_t *) slab_alloc(sysinfo_item_slab, 0);
		ASSERT(*psubtree);
		
		/* Fill in item name up to the delimiter */
		(*psubtree)->name = str_ndup(name, i);
		ASSERT((*psubtree)->name);
		
		/* Create subtree items */
		if (name[i] == '.') {
			(*psubtree)->subtree_type = SYSINFO_SUBTREE_TABLE;
			return sysinfo_create_path(name + i + 1,
			    &((*psubtree)->subtree.table));
		}
		
		/* No subtree needs to be created */
		return *psubtree;
	}
	
	sysinfo_item_t *cur = *psubtree;
	
	/* Walk all siblings */
	while (cur != NULL) {
		size_t i = 0;
		
		/* Compare name with path */
		while ((cur->name[i] != 0) && (name[i] == cur->name[i]))
			i++;
		
		/* Check for perfect name and path match
		 * -> item is already present.
		 */
		if ((name[i] == 0) && (cur->name[i] == 0))
			return cur;
		
		/* Partial match up to the delimiter */
		if ((name[i] == '.') && (cur->name[i] == 0)) {
			switch (cur->subtree_type) {
			case SYSINFO_SUBTREE_NONE:
				/* No subtree yet, create one */
				cur->subtree_type = SYSINFO_SUBTREE_TABLE;
				return sysinfo_create_path(name + i + 1,
				    &(cur->subtree.table));
			case SYSINFO_SUBTREE_TABLE:
				/* Subtree already created, add new sibling */
				return sysinfo_create_path(name + i + 1,
				    &(cur->subtree.table));
			default:
				/* Subtree items handled by a function, this
				 * cannot be overriden by a constant item.
				 */
				return NULL;
			}
		}
		
		/* No match and no more siblings to check
		 * -> create a new sibling item.
		 */
		if (cur->next == NULL) {
			/* Find the first delimiter in name */
			i = 0;
			while ((name[i] != 0) && (name[i] != '.'))
				i++;
			
			sysinfo_item_t *item =
			    (sysinfo_item_t *) slab_alloc(sysinfo_item_slab, 0);
			ASSERT(item);
			
			cur->next = item;
			
			/* Fill in item name up to the delimiter */
			item->name = str_ndup(name, i);
			ASSERT(item->name);
			
			/* Create subtree items */
			if (name[i] == '.') {
				item->subtree_type = SYSINFO_SUBTREE_TABLE;
				return sysinfo_create_path(name + i + 1,
				    &(item->subtree.table));
			}
			
			/* No subtree needs to be created */
			return item;
		}
		
		cur = cur->next;
	}
	
	/* Unreachable */
	ASSERT(false);
	return NULL;
}

/** Set sysinfo item with a constant numeric value
 *
 * @param name Sysinfo path.
 * @param root Pointer to the root item or where to store
 *             a new root item (NULL for global sysinfo root).
 * @param val  Value to store in the item.
 *
 */
void sysinfo_set_item_val(const char *name, sysinfo_item_t **root,
    sysarg_t val)
{
	/* Protect sysinfo tree consistency */
	mutex_lock(&sysinfo_lock);
	
	if (root == NULL)
		root = &global_root;
	
	sysinfo_item_t *item = sysinfo_create_path(name, root);
	if (item != NULL) {
		item->val_type = SYSINFO_VAL_VAL;
		item->val.val = val;
	}
	
	mutex_unlock(&sysinfo_lock);
}

/** Set sysinfo item with a constant binary data
 *
 * Note that sysinfo only stores the pointer to the
 * binary data and does not touch it in any way. The
 * data should be static and immortal.
 *
 * @param name Sysinfo path.
 * @param root Pointer to the root item or where to store
 *             a new root item (NULL for global sysinfo root).
 * @param data Binary data.
 * @param size Size of the binary data.
 *
 */
void sysinfo_set_item_data(const char *name, sysinfo_item_t **root,
    void *data, size_t size)
{
	/* Protect sysinfo tree consistency */
	mutex_lock(&sysinfo_lock);
	
	if (root == NULL)
		root = &global_root;
	
	sysinfo_item_t *item = sysinfo_create_path(name, root);
	if (item != NULL) {
		item->val_type = SYSINFO_VAL_DATA;
		item->val.data.data = data;
		item->val.data.size = size;
	}
	
	mutex_unlock(&sysinfo_lock);
}

/** Set sysinfo item with a generated numeric value
 *
 * @param name Sysinfo path.
 * @param root Pointer to the root item or where to store
 *             a new root item (NULL for global sysinfo root).
 * @param fn   Numeric value generator function.
 * @param data Private data.
 *
 */
void sysinfo_set_item_gen_val(const char *name, sysinfo_item_t **root,
    sysinfo_fn_val_t fn, void *data)
{
	/* Protect sysinfo tree consistency */
	mutex_lock(&sysinfo_lock);
	
	if (root == NULL)
		root = &global_root;
	
	sysinfo_item_t *item = sysinfo_create_path(name, root);
	if (item != NULL) {
		item->val_type = SYSINFO_VAL_FUNCTION_VAL;
		item->val.gen_val.fn = fn;
		item->val.gen_val.data = data;
	}
	
	mutex_unlock(&sysinfo_lock);
}

/** Set sysinfo item with a generated binary data
 *
 * Note that each time the generator function is called
 * it is supposed to return a new dynamically allocated
 * data. This data is then freed by sysinfo in the context
 * of the current sysinfo request.
 *
 * @param name Sysinfo path.
 * @param root Pointer to the root item or where to store
 *             a new root item (NULL for global sysinfo root).
 * @param fn   Binary data generator function.
 * @param data Private data.
 *
 */
void sysinfo_set_item_gen_data(const char *name, sysinfo_item_t **root,
    sysinfo_fn_data_t fn, void *data)
{
	/* Protect sysinfo tree consistency */
	mutex_lock(&sysinfo_lock);
	
	if (root == NULL)
		root = &global_root;
	
	sysinfo_item_t *item = sysinfo_create_path(name, root);
	if (item != NULL) {
		item->val_type = SYSINFO_VAL_FUNCTION_DATA;
		item->val.gen_data.fn = fn;
		item->val.gen_data.data = data;
	}
	
	mutex_unlock(&sysinfo_lock);
}

/** Set sysinfo item with an undefined value
 *
 * @param name Sysinfo path.
 * @param root Pointer to the root item or where to store
 *             a new root item (NULL for global sysinfo root).
 *
 */
void sysinfo_set_item_undefined(const char *name, sysinfo_item_t **root)
{
	/* Protect sysinfo tree consistency */
	mutex_lock(&sysinfo_lock);
	
	if (root == NULL)
		root = &global_root;
	
	sysinfo_item_t *item = sysinfo_create_path(name, root);
	if (item != NULL)
		item->val_type = SYSINFO_VAL_UNDEFINED;
	
	mutex_unlock(&sysinfo_lock);
}

/** Set sysinfo item with a generated subtree
 *
 * @param name Sysinfo path.
 * @param root Pointer to the root item or where to store
 *             a new root item (NULL for global sysinfo root).
 * @param fn   Subtree generator function.
 * @param data Private data to be passed to the generator.
 *
 */
void sysinfo_set_subtree_fn(const char *name, sysinfo_item_t **root,
    sysinfo_fn_subtree_t fn, void *data)
{
	/* Protect sysinfo tree consistency */
	mutex_lock(&sysinfo_lock);
	
	if (root == NULL)
		root = &global_root;
	
	sysinfo_item_t *item = sysinfo_create_path(name, root);
	
	/* Change the type of the subtree only if it is not already
	   a fixed subtree */
	if ((item != NULL) && (item->subtree_type != SYSINFO_SUBTREE_TABLE)) {
		item->subtree_type = SYSINFO_SUBTREE_FUNCTION;
		item->subtree.generator.fn = fn;
		item->subtree.generator.data = data;
	}
	
	mutex_unlock(&sysinfo_lock);
}

/** Sysinfo dump indentation helper routine
 *
 * @param depth Number of spaces to print.
 *
 */
NO_TRACE static void sysinfo_indent(size_t spaces)
{
	for (size_t i = 0; i < spaces; i++)
		printf(" ");
}

/** Dump the structure of sysinfo tree
 *
 * Should be called with sysinfo_lock held.
 *
 * @param root   Root item of the current (sub)tree.
 * @param spaces Current indentation level.
 *
 */
NO_TRACE static void sysinfo_dump_internal(sysinfo_item_t *root, size_t spaces)
{
	/* Walk all siblings */
	for (sysinfo_item_t *cur = root; cur; cur = cur->next) {
		size_t length;
		
		if (spaces == 0) {
			printf("%s", cur->name);
			length = str_length(cur->name);
		} else {
			sysinfo_indent(spaces);
			printf(".%s", cur->name);
			length = str_length(cur->name) + 1;
		}
		
		sysarg_t val;
		size_t size;
		
		/* Display node value and type */
		switch (cur->val_type) {
		case SYSINFO_VAL_UNDEFINED:
			printf(" [undefined]\n");
			break;
		case SYSINFO_VAL_VAL:
			printf(" -> %" PRIun" (%#" PRIxn ")\n", cur->val.val,
			    cur->val.val);
			break;
		case SYSINFO_VAL_DATA:
			printf(" (%zu bytes)\n", cur->val.data.size);
			break;
		case SYSINFO_VAL_FUNCTION_VAL:
			val = cur->val.gen_val.fn(cur, cur->val.gen_val.data);
			printf(" -> %" PRIun" (%#" PRIxn ") [generated]\n", val,
			    val);
			break;
		case SYSINFO_VAL_FUNCTION_DATA:
			/* N.B.: No data was actually returned (only a dry run) */
			(void) cur->val.gen_data.fn(cur, &size, true,
			    cur->val.gen_data.data);
			printf(" (%zu bytes) [generated]\n", size);
			break;
		default:
			printf("+ %s [unknown]\n", cur->name);
		}
		
		/* Recursivelly nest into the subtree */
		switch (cur->subtree_type) {
		case SYSINFO_SUBTREE_NONE:
			break;
		case SYSINFO_SUBTREE_TABLE:
			sysinfo_dump_internal(cur->subtree.table, spaces + length);
			break;
		case SYSINFO_SUBTREE_FUNCTION:
			sysinfo_indent(spaces + length);
			printf("<generated subtree>\n");
			break;
		default:
			sysinfo_indent(spaces + length);
			printf("<unknown subtree>\n");
		}
	}
}

/** Dump the structure of sysinfo tree
 *
 * @param root  Root item of the sysinfo (sub)tree.
 *              If it is NULL then consider the global
 *              sysinfo tree.
 *
 */
void sysinfo_dump(sysinfo_item_t *root)
{
	/* Avoid other functions to mess with sysinfo
	   while we are dumping it */
	mutex_lock(&sysinfo_lock);
	
	if (root == NULL)
		sysinfo_dump_internal(global_root, 0);
	else
		sysinfo_dump_internal(root, 0);
	
	mutex_unlock(&sysinfo_lock);
}

/** @}
 */
