#ifndef _WIFI_PROFILES_H_
#define _WIFI_PROFILES_H_

#include "c_types.h"
#include "lwip/ip_addr.h"
#include "config_flash.h"

#define WIFI_PROFILE_COUNT      6
#define WIFI_PROFILE_NONE       0xff
#define WIFI_PROFILE_BLOB_NO    1
#define WIFI_PROFILE_MAGIC      0x57494649  /* "WIFI" */

typedef struct {
    uint8_t enabled;
    uint8_t reserved[3];

    /* Same sizes as sysconfig_t. These are treated as C strings,
       so practical max lengths are 31 and 63 chars. */
    uint8_t ssid[32];
    uint8_t password[64];
} wifi_profile_t;

void wifi_profiles_load(void);
void wifi_profiles_save(void);
void wifi_profiles_zero(void);

int wifi_profiles_valid_slot(int slot);
int wifi_profiles_slot_enabled(int slot);

int wifi_profiles_add_from_config(int slot, sysconfig_t *config);
int wifi_profiles_add_explicit(int slot, const char *ssid, const char *password);
int wifi_profiles_clear(int slot);

int wifi_profiles_copy_to_config(int slot, sysconfig_t *config);
int wifi_profiles_find_match(sysconfig_t *config);

const wifi_profile_t *wifi_profiles_get(int slot);

uint8_t wifi_profiles_get_active_slot(void);
void wifi_profiles_set_active_slot(uint8_t slot);

#endif