#include "wifi_profiles.h"

#include "mem.h"
#include "osapi.h"
#include "config_flash.h"

typedef struct {
    uint32_t magic;
    uint16_t length;
    uint8_t active_slot;   /* persistent/default slot, or WIFI_PROFILE_NONE */
    uint8_t reserved;
    wifi_profile_t profile[WIFI_PROFILE_COUNT];
} wifi_profiles_blob_t;

static wifi_profiles_blob_t wifi_profiles;


static void ICACHE_FLASH_ATTR wifi_profiles_load_default(void)
{
    os_memset(&wifi_profiles, 0, sizeof(wifi_profiles));
    wifi_profiles.magic = WIFI_PROFILE_MAGIC;
    wifi_profiles.length = sizeof(wifi_profiles_blob_t);
    wifi_profiles.active_slot = WIFI_PROFILE_NONE;
}


static void ICACHE_FLASH_ATTR copy_cstr_to_fixed(uint8_t *dst, uint16_t dst_len, const char *src)
{
    uint16_t i;

    os_memset(dst, 0, dst_len);

    if (src == NULL || dst_len == 0) {
        return;
    }

    for (i = 0; i < dst_len - 1 && src[i] != 0; i++) {
        dst[i] = src[i];
    }

    dst[dst_len - 1] = 0;
}


static void ICACHE_FLASH_ATTR copy_fixed_to_fixed(uint8_t *dst,
                                                  uint16_t dst_len,
                                                  const uint8_t *src,
                                                  uint16_t src_len)
{
    uint16_t i;

    os_memset(dst, 0, dst_len);

    if (src == NULL || dst_len == 0) {
        return;
    }

    for (i = 0; i < dst_len - 1 && i < src_len && src[i] != 0; i++) {
        dst[i] = src[i];
    }

    dst[dst_len - 1] = 0;
}


void ICACHE_FLASH_ATTR wifi_profiles_load(void)
{
    blob_load(WIFI_PROFILE_BLOB_NO,
              (uint32_t *)&wifi_profiles,
              sizeof(wifi_profiles_blob_t));

    if (wifi_profiles.magic != WIFI_PROFILE_MAGIC ||
        wifi_profiles.length != sizeof(wifi_profiles_blob_t)) {
        wifi_profiles_load_default();
    }
}


void ICACHE_FLASH_ATTR wifi_profiles_save(void)
{
    wifi_profiles.magic = WIFI_PROFILE_MAGIC;
    wifi_profiles.length = sizeof(wifi_profiles_blob_t);

    blob_save(WIFI_PROFILE_BLOB_NO,
              (uint32_t *)&wifi_profiles,
              sizeof(wifi_profiles_blob_t));
}


void ICACHE_FLASH_ATTR wifi_profiles_zero(void)
{
    blob_zero(WIFI_PROFILE_BLOB_NO, sizeof(wifi_profiles_blob_t));
    wifi_profiles_load_default();
}


int ICACHE_FLASH_ATTR wifi_profiles_valid_slot(int slot)
{
    return slot >= 0 && slot < WIFI_PROFILE_COUNT;
}


int ICACHE_FLASH_ATTR wifi_profiles_slot_enabled(int slot)
{
    if (!wifi_profiles_valid_slot(slot)) {
        return 0;
    }

    return wifi_profiles.profile[slot].enabled != 0;
}


const wifi_profile_t ICACHE_FLASH_ATTR *wifi_profiles_get(int slot)
{
    if (!wifi_profiles_valid_slot(slot)) {
        return NULL;
    }

    return &wifi_profiles.profile[slot];
}


int ICACHE_FLASH_ATTR wifi_profiles_add_from_config(int slot, sysconfig_t *config)
{
    if (!wifi_profiles_valid_slot(slot) || config == NULL) {
        return -1;
    }

    os_memset(&wifi_profiles.profile[slot], 0, sizeof(wifi_profile_t));

    wifi_profiles.profile[slot].enabled = 1;

    copy_fixed_to_fixed(wifi_profiles.profile[slot].ssid,
                        sizeof(wifi_profiles.profile[slot].ssid),
                        config->ssid,
                        sizeof(config->ssid));

    copy_fixed_to_fixed(wifi_profiles.profile[slot].password,
                        sizeof(wifi_profiles.profile[slot].password),
                        config->password,
                        sizeof(config->password));

    return 0;
}


int ICACHE_FLASH_ATTR wifi_profiles_add_explicit(int slot,
                                                 const char *ssid,
                                                 const char *password)
{
    if (!wifi_profiles_valid_slot(slot) || ssid == NULL || password == NULL) {
        return -1;
    }

    os_memset(&wifi_profiles.profile[slot], 0, sizeof(wifi_profile_t));

    wifi_profiles.profile[slot].enabled = 1;

    copy_cstr_to_fixed(wifi_profiles.profile[slot].ssid,
                       sizeof(wifi_profiles.profile[slot].ssid),
                       ssid);

    copy_cstr_to_fixed(wifi_profiles.profile[slot].password,
                       sizeof(wifi_profiles.profile[slot].password),
                       password);

    return 0;
}


int ICACHE_FLASH_ATTR wifi_profiles_clear(int slot)
{
    if (!wifi_profiles_valid_slot(slot)) {
        return -1;
    }

    os_memset(&wifi_profiles.profile[slot], 0, sizeof(wifi_profile_t));

    if (wifi_profiles.active_slot == slot) {
        wifi_profiles.active_slot = WIFI_PROFILE_NONE;
    }

    return 0;
}


int ICACHE_FLASH_ATTR wifi_profiles_copy_to_config(int slot, sysconfig_t *config)
{
    if (!wifi_profiles_slot_enabled(slot) || config == NULL) {
        return -1;
    }

    copy_fixed_to_fixed(config->ssid,
                        sizeof(config->ssid),
                        wifi_profiles.profile[slot].ssid,
                        sizeof(wifi_profiles.profile[slot].ssid));

    copy_fixed_to_fixed(config->password,
                        sizeof(config->password),
                        wifi_profiles.profile[slot].password,
                        sizeof(wifi_profiles.profile[slot].password));

    return 0;
}


int ICACHE_FLASH_ATTR wifi_profiles_find_match(sysconfig_t *config)
{
    int i;

    if (config == NULL) {
        return -1;
    }

    for (i = 0; i < WIFI_PROFILE_COUNT; i++) {
        if (!wifi_profiles_slot_enabled(i)) {
            continue;
        }

        if (strcmp((char *)config->ssid, (char *)wifi_profiles.profile[i].ssid) == 0 &&
            strcmp((char *)config->password, (char *)wifi_profiles.profile[i].password) == 0) {
            return i;
        }
    }

    return -1;
}


uint8_t ICACHE_FLASH_ATTR wifi_profiles_get_active_slot(void)
{
    return wifi_profiles.active_slot;
}


void ICACHE_FLASH_ATTR wifi_profiles_set_active_slot(uint8_t slot)
{
    if (slot == WIFI_PROFILE_NONE || slot < WIFI_PROFILE_COUNT) {
        wifi_profiles.active_slot = slot;
    } else {
        wifi_profiles.active_slot = WIFI_PROFILE_NONE;
    }
}