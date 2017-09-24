#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "hash.h"

/* rehash, if max_node reached */
static const struct hash_spec_st {
    unsigned int nslot;
    unsigned int max_node;
} hash_specs[] = {
    { (1<<3), 24 },
    { (1<<4), 64 },
    { (1<<6), 320 },
    { (1<<8), 1536 },
    { (1<<10), 7168 },
    { (1<<12), 2147483647 }
};

static void *zero_alloc(int size)
{
    void *r = malloc(size);
    if (!r) {
        fprintf(stderr, "Out of memory\n");
        return NULL;
    }
    memset(r, 0, size);
    return r;
}


/* the default key function, famous time33 alg */
static unsigned int hash_default_key_time33(const void *key, int klen)
{
    unsigned int h = 5318;
    const unsigned char *p = (const unsigned char *)key;

    while (klen > 0) {
        h = h * 33 + (*p);
        p++;
        klen--;
    }
    return h;
}

/* create an hash, with data function and key generate function, all can be NULL */
hash_st *hash_create(hash_data_free_func_t del, hash_key_func_t keyf)
{
    hash_st *h = zero_alloc(sizeof(hash_st));

    h->nslot = hash_specs[0].nslot;
    h->max_element = hash_specs[0].max_node;
    h->slots = zero_alloc(h->nslot * sizeof(struct hash_node_st *));

    h->hdel = del;

    if (keyf)
      h->hkey = keyf;
    else
      h->hkey = hash_default_key_time33;

    return h;
}

/* extern the slots of hash and rearrange all nodes */
static void hash_rehash(hash_st *h)
{
    unsigned new_nslot = 0, new_max = 0;
    struct hash_node_st **new_slots;
    unsigned int i;

    /* finding the next slot size and max element */
    for (i = 0; i < sizeof(hash_specs)/sizeof(struct hash_spec_st) - 1; ++i) {
        if (hash_specs[i].nslot == h->nslot) {
            new_nslot = hash_specs[i+1].nslot;
            new_max = hash_specs[i+1].max_node;
            break;
        }
    }
    if (!new_nslot) {
        fprintf(stderr, "[BUG] rehashing, can not get next_mask\n");
        return;
    }

    /* allocate new slots and move every node to new slots */
    new_slots = zero_alloc(new_nslot * sizeof(struct hash_node_st *));

    for (i = 0; i < h->nslot; ++i) {
        struct hash_node_st *p = h->slots[i];
        while (p) {
            struct hash_node_st *next = p->next;
            unsigned int newindex = p->__hval & (new_nslot - 1);

            p->next = new_slots[newindex];
            new_slots[newindex] = p;

            p = next;
        }
    }
    free(h->slots);
    h->slots = new_slots;
    h->nslot = new_nslot;
    h->max_element = new_max;
}

/* insert, if key is exist, an del function will be call on the old data, and replace with new data */
void hash_insert(hash_st *ht, const void *key, int len, void *val)
{
    unsigned int hval = ht->hkey(key, len);
    unsigned int idx = hval & (ht->nslot - 1);
    struct hash_node_st *tmp;
    struct hash_node_st *p = ht->slots[idx];	

    /* lookup whether the node exist, replace then */
    while (p) {
        if (hval == p->__hval && p->klen == len && memcmp(p->key, key, len) == 0) {
            if ((void *)ht->hdel) 
              ht->hdel(p->val);

            p->val = val;
            return;
        }
        p = p->next;
    }

    /* new node for insert */
    tmp = malloc(sizeof(struct hash_node_st));
    if (!tmp)
      fprintf(stderr, "out of memory\n");

    tmp->key = malloc(len);
    if (!tmp->key)
      fprintf(stderr, "out of memory\n");

    tmp->klen = len;
    memcpy(tmp->key, key, len);

    tmp->val = val;
    tmp->__hval = hval;

    tmp->next = ht->slots[idx];
    ht->slots[idx] = tmp;
    ht->nelement++;

    if (ht->nelement >= ht->max_element)			/* rehash after newly node inserted */
      hash_rehash(ht);
}

/* search the key, return at void **val return 0 if key founded */
int hash_search(hash_st *ht, const void *key, int len, void **val)
{
    unsigned int hval = ht->hkey(key, len);
    unsigned int idx = hval & (ht->nslot - 1);
    struct hash_node_st *p = ht->slots[idx];

    while(p) {
        if (hval == p->__hval && p->klen == len && memcmp(p->key, key, len) == 0) {
            if (val)
              *val = p->val;

            return 0;
        }
        p = p->next;
    }
    return -1;
}

/* delete the key of the hash, del function will be called on this element */
int hash_delete(hash_st *ht, const void *key, int len)
{
    unsigned int hval = ht->hkey(key, len);
    unsigned int idx = hval & (ht->nslot - 1);
    struct hash_node_st *p = ht->slots[idx];
    struct hash_node_st *last = NULL;

    while (p) {
        if (hval == p->__hval && p->klen == len && memcmp(p->key, key, len) == 0) {
            if (last)
              last->next = p->next;
            else
              ht->slots[idx] = p->next;

            ht->nelement--;

            if ((void *)ht->hdel) 
              ht->hdel(p->val);

            free(p->key);
            free(p);

            return 0;
        }
        last = p;
        p = p->next;
    }
    return -1;
}

/* destroy every thing */
void hash_destroy(hash_st *ht)
{
    unsigned int i;
    struct hash_node_st *t;

    for(i = 0; i < ht->nslot; i++) {
        while(ht->slots[i]) {
            t = ht->slots[i];
            ht->slots[i] = ht->slots[i]->next;

            if ((void *)ht->hdel) 
              ht->hdel(t->val);

            free(t->key);
            free(t);
        }
    }
    free(ht->slots);
    free(ht);
}

/* walk the hash with callback on every element */
int hash_walk(hash_st *ht, void *udata, hash_walk_func_t fn)
{
    struct hash_node_st *p;
    unsigned int i;

    if (!(void *)fn)
      return -1;

    for (i = 0; i < ht->nslot; ++i) {
        p = ht->slots[i];
        while (p) {
            struct hash_node_st *next = p->next;

            if (fn(p->key, p->klen, p->val, udata))
              return 1;

            p = next;
        }
    }
    return 0;
}

#ifdef __HASH_TEST__

#include "strutils.h"

const char *search[6] = {
    "123abc",
    "456abc",
    "789abc",
    "123abcd",
    "23opiu023",
    "10086abc"
};

int main(int argc, char **argv)
{
    int i;
    hash_st *h = hash_create(NULL, NULL);

    for (i = 0; i < 100000; ++i) {
        char buf[100];
        int n = str_snprintf(buf, 100, "%dabc", i);

        hash_insert(h, buf, n, (void *)i);
    }


    hash_delete(h, search[0], strlen(search[0]));

    void *k;
    for (i = 0; i < 4; ++i) {
        if (hash_search(h, search[i], strlen(search[i]), &k) == 0) {
            printf("found %s in %d\n", search[i], (int)k);
        }
    }

    for (i = 0; i < 100000; ++i) {
        char buf[100];
        int n = str_snprintf(buf, 100, "%dabc", i);

        hash_delete(h, buf, n);
    }

    hash_destroy(h);
    return 0;
}

#endif

