#include <stdio.h>

#include "domain.h"

int main()
{
    load_root_domain_conf();
    char *hostname = "api.test.www.baidu.com.cn";
    printf("hostname is:%s\n", hostname);
    char domain[126] = {0};
    get_main_domain(hostname, domain);
    printf("main domain is:%s\n", domain);

    char *hostname1 = "baidu.com.cn";
    printf("hostname is:%s\n", hostname1);
    char domain1[126] = {0};
    get_main_domain(hostname1, domain1);
    printf("main domain is:%s\n", domain1);

    char *hostname2 = "baidu.com";
    printf("hostname is:%s\n", hostname2);
    char domain2[126] = {0};
    get_main_domain(hostname2, domain2);
    printf("main domain is:%s\n", domain2);

    char *hostname3 = "www.gzck.cn";
    printf("hostname is:%s\n", hostname3);
    char domain3[126] = {0};
    get_main_domain(hostname3, domain3);
    printf("main domain is:%s\n", domain3);

    char *hostname4 = "www.jtyhjy.cn";
    printf("hostname is:%s\n", hostname4);
    char domain4[126] = {0};
    get_main_domain(hostname4, domain4);
    printf("main domain is:%s\n", domain4);

    unload_domain_conf();
    return 0;
}
