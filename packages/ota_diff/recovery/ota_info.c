#include "ota_info.h"
#include "crc8.h"
#include <stdint.h>
#include <flashdb.h>

#define OTA_KV_BLOB_NEW_VERSION      "kv_blob_new_version"
#define OTA_KV_BLOB_CURRENT_VERSION  "kv_blob_current_version"
#define OTA_ACTIVE_APP_PARTITION     "active_app"
#define OTA_BACKUP_APP_PARTITION     "backup_app"
#define OTA_PATCH_PARTITION     	 "patch"


static struct fdb_default_kv_node default_kv_set[] = {
        {"new_version", "0"},
        {"current_version", "0"},
        {"jump_address", "0"},
        {"ota_support", "0"},
};

static struct fdb_kvdb ota_kvdb;
static ota_info_tag_t default_ota_info;
static ota_info_tag_t *p_ota_info = NULL;


static void ota_fdb_kvdb_init(void)
{
    struct fdb_default_kv default_kv;

    default_kv.kvs = default_kv_set;
    default_kv.num = sizeof(default_kv_set) / sizeof(default_kv_set[0]);
    if( fdb_kvdb_init(&ota_kvdb, "kv", "flashdb", &default_kv, NULL) != FDB_NO_ERR)
    {
		rt_kprintf("create flashDB failed.\r\n");
	}
}



int ota_info_init(void)
{
	uint8_t crc = 0;
	size_t version_len;
    size_t read_len;
	fdb_err_t result;
    struct fdb_kv kv_obj;
    struct fdb_blob blob;
	ota_info_tag_t ota_info;
	ota_img_hdr_t img_hdr;
	const struct fal_flash_dev *flash_dev;
	ota_img_vs_t new_version;

	/* flashDB initialized */
	ota_fdb_kvdb_init();

    ota_info.active_app_partition = fal_partition_find(OTA_ACTIVE_APP_PARTITION);
    if (!ota_info.active_app_partition) {
        return -1;
    }

	/* get jump address */
	flash_dev = fal_flash_device_find(ota_info.active_app_partition->flash_name);
	if (!flash_dev) {
		return -1;	
	}
	ota_info.jump_address = flash_dev->addr + 
		ota_info.active_app_partition->offset;

    ota_info.patch_partition = fal_partition_find(OTA_PATCH_PARTITION);
    if (!ota_info.backup_app_partition) {
        return -1;
    }

    ota_info.backup_app_partition = fal_partition_find(OTA_BACKUP_APP_PARTITION);
    if (ota_info.backup_app_partition) {
        ota_info.ota_type = OTA_UPDATE_PING_PONG;
    } else {
		ota_info.ota_type = OTA_UPDATE_IN_POSITION;
	}


    read_len = fdb_kv_get_blob(&ota_kvdb, OTA_KV_BLOB_CURRENT_VERSION, 
		fdb_blob_make(&blob, &new_version, sizeof(ota_img_vs_t)));
	if (read_len != sizeof(ota_img_vs_t)) {
		new_version.major = 0;
		new_version.minor = 1;
		read_len = fdb_kv_set_blob(&ota_kvdb, OTA_KV_BLOB_CURRENT_VERSION, 
			fdb_blob_make(&blob, &new_version, sizeof(ota_img_vs_t)));
		if (read_len == sizeof(ota_img_vs_t)) {
			return -1;
		}
	}
	ota_info.current_version = new_version;/* invalid new version */

	/* new image detect */    
    read_len = fdb_kv_get_blob(&ota_kvdb, OTA_KV_BLOB_NEW_VERSION, 
		fdb_blob_make(&blob, &new_version, sizeof(ota_img_vs_t)));
	if (read_len == sizeof(ota_img_vs_t)) {
		ota_info.new_version = new_version;
	} else {
		/* no new image,make sure new version is invalid */
		new_version.major = 0;
		new_version.minor = 0;
        
        read_len = fdb_kv_set_blob(&ota_kvdb, OTA_KV_BLOB_NEW_VERSION, 
			fdb_blob_make(&blob, &new_version, sizeof(ota_img_vs_t)));
		if (read_len == sizeof(ota_img_vs_t)) {
			return -1;
		}
	}

	default_ota_info = ota_info;
	p_ota_info = &default_ota_info;
	
	return 0;
}

ota_info_tag_t *ota_info_get(void)
{
	return p_ota_info;
}



ota_err_t ota_info_current_version_get(ota_img_vs_t *new_version)
{
    fdb_err_t result = FDB_NO_ERR;
    size_t read_len;
    struct fdb_blob blob;

    read_len = fdb_kv_get_blob(&ota_kvdb, OTA_KV_BLOB_CURRENT_VERSION, 
    	fdb_blob_make(&blob, new_version, sizeof(ota_img_vs_t)));
    if( blob.saved.len == 0 || read_len == 0) {
		return FDB_READ_ERR;
	}

    return FDB_NO_ERR;
}



ota_err_t ota_info_new_version_get(ota_img_vs_t *new_version)
{
    fdb_err_t result = FDB_NO_ERR;
    size_t read_len;
    struct fdb_blob blob;

    read_len = fdb_kv_get_blob(&ota_kvdb, OTA_KV_BLOB_NEW_VERSION, 
    	fdb_blob_make(&blob, new_version, sizeof(ota_img_vs_t)));
    if( blob.saved.len == 0 || read_len == 0) {
		return FDB_READ_ERR;
	}

    return FDB_NO_ERR;
}


ota_err_t ota_info_current_version_update(ota_img_vs_t *current_version)
{
    fdb_err_t result = FDB_NO_ERR;
    size_t read_len;
    struct fdb_blob blob;
	ota_img_vs_t readback_version;

    result = fdb_kv_set_blob(&ota_kvdb, OTA_KV_BLOB_CURRENT_VERSION, 
		fdb_blob_make(&blob, current_version, sizeof(ota_img_vs_t)));
    if ( result != FDB_NO_ERR) {
		return FDB_WRITE_ERR;
	}

    read_len = fdb_kv_get_blob(&ota_kvdb, OTA_KV_BLOB_CURRENT_VERSION, 
    	fdb_blob_make(&blob, &readback_version, sizeof(ota_img_vs_t)));
    if( read_len != sizeof(ota_img_vs_t) || 
		memcmp(current_version,&readback_version, sizeof(ota_img_vs_t)) ) {
		return FDB_READ_ERR;
	}

    return FDB_NO_ERR;
}


ota_err_t ota_info_new_version_update(ota_img_vs_t *new_version)
{
    fdb_err_t result = FDB_NO_ERR;
    size_t read_len;
    struct fdb_blob blob;
	ota_img_vs_t readback_version;

    result = fdb_kv_set_blob(&ota_kvdb, OTA_KV_BLOB_NEW_VERSION, 
		fdb_blob_make(&blob, new_version, sizeof(ota_img_vs_t)));
    if ( result != FDB_NO_ERR) {
		return FDB_WRITE_ERR;
	}

    read_len = fdb_kv_get_blob(&ota_kvdb, OTA_KV_BLOB_NEW_VERSION, 
    	fdb_blob_make(&blob, &readback_version, sizeof(ota_img_vs_t)));
    if( read_len != sizeof(ota_img_vs_t) || 
		memcmp(new_version,&readback_version, sizeof(ota_img_vs_t)) ) {
		return FDB_READ_ERR;
	}

    return FDB_NO_ERR;
}




/* default:OTA_UPDATE_IN_POSITION */
int ota_partition_is_pingpong(void)
{
	if (p_ota_info ) {
		return (p_ota_info->ota_type == OTA_UPDATE_PING_PONG);
	}
	
    return 0;
}


