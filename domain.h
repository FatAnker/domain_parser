#ifndef __DOMAIN_H__
#define __DOMAIN_H__

int load_root_domain_conf();

int unload_domain_conf();

int get_main_domain(const char *hostname, char *domain);

#endif /*__DOMAIN_H__*/
