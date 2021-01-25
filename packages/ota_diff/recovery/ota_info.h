#ifndef __OTA_INFO__
#define __OTA_INFO__

#include <stdint.h>
#include "ota_err.h"

#define OTA_IMAGE_MAGIC             0xBADE


typedef enum ota_update_type_en {
    OTA_UPDATE_IN_POSITION,
    OTA_UPDATE_PING_PONG,
} ota_updt_type_t;


typedef enum ota_partition_type_en {
    OTA_PARTITION_ACTIVE_APP,
    OTA_PARTITION_BACKUP_APP,
    OTA_PARTITION_OTA,
    OTA_PARTITION_KV,
} ota_pt_type_t;


typedef struct ota_image_version_st {
    uint8_t     major;  /* major version number */
    uint8_t     minor;  /* minor version number */
} ota_img_vs_t;


#pragma pack(push, 1)
typedef struct ota_image_header_st {
    uint16_t            magic;
    ota_img_vs_t        new_version;
    ota_img_vs_t        old_version;

    uint8_t             new_crc;
    uint8_t             old_crc;
    uint8_t             patch_crc;

    uint32_t            new_size;
    uint32_t            old_size;
    uint32_t            patch_size;
} ota_img_hdr_t;
#pragma pack(pop)


typedef struct ota_info_tag {
    ota_pt_type_t     ota_type;  /* support ota type */
    ota_img_vs_t      new_version; /* new image version */
    ota_img_vs_t      current_version; /* current image version */
	uint32_t		  jump_address;	/* application location */
	const struct fal_partition *active_app_partition; 
								/* active application partition on flash */
	const struct fal_partition *backup_app_partition; 
								/* backup application partition on flash */
	const struct fal_partition *patch_partition; 
								/* patch partition on flash */
} ota_info_tag_t;


int ota_info_init(void);
ota_info_tag_t *ota_info_get(void);
ota_err_t ota_info_current_version_get(ota_img_vs_t *new_version);
ota_err_t ota_info_new_version_get(ota_img_vs_t *new_version);
ota_err_t ota_info_current_version_update(ota_img_vs_t *current_version);
ota_err_t ota_info_new_version_update(ota_img_vs_t *new_version);
int ota_partition_is_pingpong(void);



#endif //__OTA_INFO__

