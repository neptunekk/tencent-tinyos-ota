/*----------------------------------------------------------------------------
 * Tencent is pleased to support the open source community by making TencentOS
 * available.
 *
 * Copyright (C) 2019 THL A29 Limited, a Tencent company. All rights reserved.
 * If you have downloaded a copy of the TencentOS binary from Tencent, please
 * note that the TencentOS binary is licensed under the BSD 3-Clause License.
 *
 * If you have downloaded a copy of the TencentOS source code from Tencent,
 * please note that TencentOS source code is licensed under the BSD 3-Clause
 * License, except for the third-party components listed below which are
 * subject to different license terms. Your integration of TencentOS into your
 * own projects may require compliance with the BSD 3-Clause License, as well
 * as the other licenses applicable to the third-party components included
 * within TencentOS.
 *---------------------------------------------------------------------------*/

#include "crc8.h"
#include "ota_info.h"
#include "lzma_uncompress.h"
#include "ota_recovery.h"
#include <fal.h>
#include <flashdb.h>

#define BUF_SIZE        2048
#define MIN(a, b)       ((a) < (b) ? (a) : (b))


uint8_t ota_img_hdr_crc(ota_img_hdr_t *img_hdr)
{
    uint8_t crc = 0;

    crc = crc8(crc, (uint8_t *)&img_hdr->magic, sizeof(uint16_t));
    crc = crc8(crc, (uint8_t *)&img_hdr->new_version, sizeof(ota_img_vs_t));
    crc = crc8(crc, (uint8_t *)&img_hdr->old_version, sizeof(ota_img_vs_t));

    crc = crc8(crc, (uint8_t *)&img_hdr->new_crc, sizeof(uint8_t));
    crc = crc8(crc, (uint8_t *)&img_hdr->new_size, sizeof(uint32_t));

    crc = crc8(crc, (uint8_t *)&img_hdr->old_crc, sizeof(uint8_t));
    crc = crc8(crc, (uint8_t *)&img_hdr->old_size, sizeof(uint32_t));

    crc = crc8(crc, (uint8_t *)&img_hdr->patch_size, sizeof(uint32_t));

    return crc;
}

static ota_err_t patch_verify(ota_img_hdr_t *img_hdr)
{
	ota_info_tag_t *ota_info;
    uint8_t *buf;
    uint8_t crc = 0;
    ota_img_vs_t new_version;
    ota_img_vs_t cur_version;
    size_t remain_len, read_len;
	ota_err_t result = OTA_ERR_PATCH_READ_FAIL;
	uint32_t offset;

	ota_info = ota_info_get();
	if (!ota_info) {
		return OTA_ERR_PATCH_READ_FAIL;
	}
	
    buf = malloc(BUF_SIZE);
    if (!buf) {
        return OTA_ERR_PATCH_READ_FAIL;
    }

    /* drag the ota_img_hdr out of the flash */
	if (fal_partition_read(ota_info->patch_partition, 0, 
		(uint8_t *)img_hdr, sizeof(ota_img_hdr_t) ) < 0) {
		result = OTA_ERR_PATCH_READ_FAIL;
		goto __exit;

	}

    /* 1. check whether new version patch downloaded */
    if (ota_info_new_version_get(&new_version) != FDB_NO_ERR) {
        result = OTA_ERR_KV_GET_FAIL;
		goto __exit;
    }

    if (new_version.major != img_hdr->new_version.major ||
        new_version.minor != img_hdr->new_version.minor) {
        result = OTA_ERR_NEW_VERSION_NOT_SAME;
        goto __exit;
    }

    /* 2. verify magic */
    if (img_hdr->magic != OTA_IMAGE_MAGIC) {
        result = OTA_ERR_PATCH_MAGIC_NOT_SAME;
        goto __exit;
    }

    /* 3. is this patch for current version? */
    if (ota_info_current_version_get(&cur_version) != FDB_NO_ERR) {
        result = OTA_ERR_KV_GET_FAIL;
        goto __exit;
    }
    if (cur_version.major != img_hdr->old_version.major ||
        cur_version.minor != img_hdr->old_version.minor) {
        result = OTA_ERR_OLD_VERSION_NOT_SAME;
        goto __exit;
    }

    /* 4. verify the patch crc checksum */
    crc             = ota_img_hdr_crc(img_hdr);
    remain_len      = img_hdr->patch_size;
	offset			= sizeof(ota_img_hdr_t);

    while (remain_len > 0) {
        read_len = MIN(BUF_SIZE, remain_len);
        if (fal_partition_read(ota_info->patch_partition, offset, buf, read_len) < 0) {
            result = OTA_ERR_PATCH_READ_FAIL;
            goto __exit;
        }

        crc = crc8(crc, buf, read_len);

        remain_len     -= read_len;
        offset    	   += read_len;
    }

    if (crc != img_hdr->patch_crc) {
        result = OTA_ERR_PATCH_CRC_FAIL;
        goto __exit;
    }

    /* 5. verify the old crc checksum */
    remain_len      = img_hdr->old_size;
    crc             = 0;
	offset			= 0;

    while (remain_len > 0) {
        read_len = MIN(BUF_SIZE, remain_len);
        if (fal_partition_read(ota_info->active_app_partition, offset, buf, read_len) < 0) {
            result = OTA_ERR_ACTIVE_APP_READ_FAIL;
            goto __exit;
        }

        crc = crc8(crc, buf, read_len);

        remain_len         -= read_len;
        offset             += read_len;
    }

    if (crc != img_hdr->old_crc) {
        result = OTA_ERR_ACTIVE_APP_CRC_FAIL;
        goto __exit;
    }

	result = OTA_ERR_NONE;

__exit:
    free(buf);
    return result;
}

static int do_recovery(ota_img_hdr_t *hdr)
{
    uint32_t patch = sizeof(ota_img_hdr_t);

    return ota_patch(patch, hdr->patch_size - sizeof(ota_img_hdr_t), hdr->patch_size);
}

static int partition_recovery(
	const struct fal_partition *dst_partition,
	const struct fal_partition *src_partition)
{
	int ret = -1;
    uint8_t *buf;
    size_t read_len = 0;
    size_t remain_len,offset;

    if ( !dst_partition ||
		 !src_partition) {
        return -1;
    }

	if (dst_partition->len != src_partition->len) {
		return -1;
	}

	buf = malloc(BUF_SIZE);
	if (!buf) {
		return -1;
	}

    /* make flash ready */
    if (fal_partition_erase(dst_partition, 0, dst_partition->len) < 0) {
        goto __exit;
    }

	offset = 0;
	remain_len = dst_partition->len;
    while (remain_len > 0) {
        read_len = MIN(BUF_SIZE, remain_len);
        if (fal_partition_read(src_partition, offset, buf, read_len) < 0) {
            goto __exit;
        }

        if (fal_partition_write(dst_partition, offset, buf, read_len) < 0) {
            goto __exit;
        }

		offset += read_len;
        remain_len  -= read_len;
    }
    ret = 0;

__exit:
	free(buf);
    return ret;
}





ota_err_t ota_recovery(void)
{
    ota_err_t ret;
    ota_img_hdr_t img_hdr;
	ota_info_tag_t *ota_info;

	ota_info = ota_info_get();
	if (!ota_info) {
		return OTA_ERR_FLASH_INIT_FAIL;
	}

    if ((ret = patch_verify(&img_hdr)) != OTA_ERR_NONE) {
        return ret;
    }

    /* backup */
    if (ota_partition_is_pingpong() &&
		partition_recovery(ota_info->backup_app_partition,
			ota_info->active_app_partition) < 0 ) {
        return OTA_ERR_BACK_UP_FAIL;
    }

    if (do_recovery(&img_hdr) != 0) {

        /* restore */
        if (ota_partition_is_pingpong()) {
            partition_recovery(ota_info->active_app_partition,
				ota_info->backup_app_partition);
        }
        return OTA_ERR_RECOVERRY_FAIL;
    }

    if ((ret = ota_info_new_version_update(&img_hdr.new_version)) != OTA_ERR_NONE) {
        return ret;
    }

    return OTA_ERR_NONE;
}

