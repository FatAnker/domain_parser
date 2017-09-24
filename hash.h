#ifndef __HASH_H__
#define __HASH_H__

struct hash_node_st {
	void *key;
	int klen;					/* key and len */
	unsigned int __hval;		/* hash value of this node, for rehashing */
	void *val;					/* value */
	struct hash_node_st *next;
};

/**
 * 释放哈希表中data字段的函数类型
 * 释放函数不应该对当前hash进行增删操作，否则会出现不必要的问题
 * \param data 数据指针
 */
typedef void (*hash_data_free_func_t)(void *data);

/** 
 * 哈希函数类型
 * \param key 哈希表的key
 * \param klen key的长度
 */
typedef unsigned int (*hash_key_func_t)(const void *key, int klen);

/**
 * 哈希表结构
 */
typedef struct hash_st {
	struct hash_node_st **slots;
	unsigned int nslot;			/* number of bucket */
	unsigned int max_element;	/* max element allowed for next rehashing */
	unsigned int nelement;		/* hash entry count */
	hash_data_free_func_t hdel;
	hash_key_func_t hkey;
} hash_st;

/**
 * 取得哈希表项数
 */
#define hash_count(ha)	((ha)->nelement)

/**
 * 创建一个哈希表
 * 使用hash_destroy释放创建的哈希表
 * \param del 释放data的回调函数，data释放将在哈希表销毁或key被替换时执行，可以为空（默认不释放）
 * \param keyf 使用自定义的哈希函数，可以为空（默认使用time33对字节进行计算）
 * \retval NULL 创建失败
 * \retval !NULL 哈希表结构指针
 */
hash_st *hash_create(hash_data_free_func_t del, hash_key_func_t keyf);

/**
 * 哈希表插入
 * \note 如果插入的key存在，新的val将替换已存在的数据，被替换的数据将调用哈希表data释放回调进行释放
 * \param ht 哈希表结构
 * \param key key字段
 * \param klen key的长度
 * \param val 插入的数据
 */
void hash_insert(hash_st *ht, const void *key, int klen, void *val);

/**
 * 哈希表查找
 * \param ht 哈希表结构
 * \param key key字段
 * \param klen key的长度
 * \param [out] val 返回数据指针
 * \retval 0 成功查找
 * \retval !0 无法找到
 */
int hash_search(hash_st *ht, const void *key, int klen, void **val);

/**
 * 哈希表删除键
 * \note 删除的数据将调用data释放回调进行释放
 * \param ht 哈希表结构
 * \param key key字段
 * \param len key的长度
 * \retval 0 成功删除
 * \retval !0 无法找到键值
 */
int hash_delete(hash_st *ht, const void *key, int len);

/**
 * 哈希表释放
 * \note 所有数据将调用data释放回调进行释放
 * \param ht 哈希表结构
 */
void hash_destroy(hash_st *ht);

/**
 * 哈希表遍历函数类型，遍历时将在每个key-value上调用该函数
 * \param key 索引字段
 * \param klen 索引字段长度
 * \param val 值指针
 * \param data 自定义数据
 * \return 正常必须返回0，否则将引发遍历函数返回该值
 */
typedef int (*hash_walk_func_t)(const void *key, int klen, void *val, void *data);

/**
 * 哈希表遍历，对所有的键值对调用fn回调
 * \param ht 哈希表结构
 * \param data 用户回调数据
 * \param fn 对每个键值对调用该回调，如果回调返回非0值，立即结束遍历并将该值返回
 * \retval 0 成功进行遍历操作
 * \retval !0 回调返回了非0值
 */
int hash_walk(hash_st *ht, void *data, hash_walk_func_t fn);

#endif
