#ifndef _INIT_BOOTENV_H
#define _INIT_BOOTENV_H

#ifdef __cplusplus
extern "C" {
#endif

int bootenv_init();
int bootenv_reinit();
const char * bootenv_get(const char * key);
int bootenv_update(const char* name, const char* value);
void bootenv_print(void);


#if BOOT_ARGS_CHECK
void 	check_boot_args();
#endif

#ifdef __cplusplus
}
#endif
#endif

