#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "domain.h"
#include "hash.h"

#define ROOT_DOMAIN_CONF_PATH   "./suffixs.txt" 
#define MAX_LINE                256
#define MAX_DOMAIN_LEN          256
#define DOT_MAX_CNT             16

static struct hash_st * s_root_domain_hash = NULL;

static void hash_data_free(void *data)
{
    if (data != NULL) {
        free(data);
        data = NULL;
    }
}

static int get_root_domain_dot_count(const char *root_domain) 
{
    int dot_cnt = 0;
    int i = 0;

    for (i = 0; i < strlen(root_domain); i++) {
        if (root_domain[i] == '.') {
            dot_cnt++;
        }
    }

    return dot_cnt;
}

/*计算hostname中.的个数及位置*/
static int s_parse_hostname_dot_pos(const char *hostname, int *dot_array)
{
    if (hostname == NULL || dot_array == NULL) {
        return -1;
    }

    int i = 0;
    int count = 0;

    for (i = 0; i < strlen(hostname); i++) {
        if (hostname[i] == '.') {
            dot_array[count] = i;
            count++;
        }
    }
    return count;
}

int load_root_domain_conf()
{
    s_root_domain_hash = hash_create(hash_data_free, NULL);
    if (s_root_domain_hash == NULL) {
        return -1;
    }

    int dot_cnt = 0;
    int *data = NULL;
    FILE *fp;   
    char line[MAX_LINE] = {0};                             //读取缓冲区  
    char root_domain[MAX_DOMAIN_LEN] = {0};

    if((fp = fopen(ROOT_DOMAIN_CONF_PATH, "r")) == NULL) {   
        fprintf(stderr, "Open Falied!");   
        return -1;   
    }   

    while (!feof(fp)) {   
        if (fgets(line, MAX_LINE, fp) == NULL) {
            break;
        }
        memset(root_domain, 0, MAX_DOMAIN_LEN);
        sscanf(line, "%s", root_domain);
        dot_cnt = get_root_domain_dot_count(root_domain);
        data = (int *)malloc(sizeof(int));
        *data = dot_cnt;
        hash_insert(s_root_domain_hash, root_domain, strlen(root_domain), (void *)data);
    }

    fclose(fp);

    return 0;
}

int unload_domain_conf()
{
    if (s_root_domain_hash) {
        hash_destroy(s_root_domain_hash);
        s_root_domain_hash = NULL;
    }
    return 0;
}

int get_main_domain(const char *hostname, char *domain) 
{
    if (hostname == NULL || domain == NULL) {
        return -1;
    }

    void *data = NULL;
    int dot_array[DOT_MAX_CNT] = {0};
    int dot_cnt = s_parse_hostname_dot_pos(hostname, dot_array);
    char root_domain[MAX_DOMAIN_LEN] = {0};
    char last_domain[MAX_DOMAIN_LEN] = {0};

    if (dot_cnt == 1) {
        memcpy(domain, hostname, strlen(hostname));
        return 0;
    } 

    /*构造.com.cn顶级域名*/
    strncpy(root_domain, hostname+dot_array[dot_cnt-2]+1, dot_array[dot_cnt-1] - dot_array[dot_cnt-2]-1); 
    strncpy(last_domain, hostname+dot_array[dot_cnt-1], strlen(hostname) - dot_array[dot_cnt-1]); 

    strcat(root_domain, last_domain);
    if (hash_search(s_root_domain_hash, root_domain, strlen(root_domain), &data) == 0) {
        if (dot_cnt == 2) {
            strncpy(domain, hostname, dot_array[0]+1);
        } else {
            strncpy(domain, hostname+dot_array[dot_cnt-3]+1, dot_array[dot_cnt-2]-dot_array[dot_cnt-3]);
        }
        strcat(domain, root_domain);
        return 0;
    } 

    /*构造.com顶级域名*/
    memset(last_domain, 0, MAX_DOMAIN_LEN);
    strncpy(last_domain, hostname+dot_array[dot_cnt-1]+1, strlen(hostname) - dot_array[dot_cnt-1]-1); 
    if (hash_search(s_root_domain_hash, last_domain, strlen(last_domain), &data) == 0) {
        memcpy(domain, root_domain, strlen(root_domain));
        return 0;
    }

    return -1;
}
