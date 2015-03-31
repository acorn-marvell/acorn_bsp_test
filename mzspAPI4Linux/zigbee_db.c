/*
 *  Copyright 2008-2013, Marvell International Ltd.
 *  All Rights Reserved.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "inc/common.h"
#include "inc/cli_utils.h"
#include "inc/zigbee.h"
#include "inc/mzspAPI.h"

#define MAX_NODE_NUM  50
#define MAX_ZONE_NUM  16
#define MAX_SCENE_NUM 16

static struct node_info nodes[MAX_NODE_NUM];
static uint32_t num_nodes; 

static struct zone_info zones[MAX_ZONE_NUM];
static uint32_t num_zones; 

static struct scene_info scenes[MAX_SCENE_NUM];
static uint32_t num_scenes; 

int get_node_indfo_pos(uint32_t *idptr, int *isFound)
{
    int i;
    for (i = 0; i < num_nodes; i++) {
        if (nodes[i].id_l[0] == idptr[0] && nodes[i].id_l[1] == idptr[1]) {
            *isFound = 1;
            return i; 
        }
    }

    if (i == MAX_NODE_NUM)
        return -1;

    return i;
}

int get_node_indfo_saddr_pos(uint16_t id_s)
{
    int i;
    for (i = 0; i < num_nodes; i++) {
        if (nodes[i].id_s == id_s) {
            return i;
        }
    }

    return -1;
}

int get_zone_indfo_pos(uint16_t id)
{
    int i;
    for (i = 0; i < num_zones; i++) {
        if (zones[i].id == id) {
            return i; 
        }
    }

    if (i == MAX_ZONE_NUM)
        return -1;

    return i;
}

int get_scene_indfo_pos(uint16_t id)
{
    int i;
    for (i = 0; i < num_scenes; i++) {
        if (scenes[i].id == id) {
            return i; 
        }
    }

    if (i == MAX_SCENE_NUM)
        return -1;

    return i;
}

static uint32_t array_node_info_add(uint16_t shortAddress, uint8_t *ieeeAddress)
{
    int isFound = 0;
    int pos;
    uint32_t id_l[2];

    id_l[0] = *((uint32_t *)ieeeAddress+1);
    id_l[1] = *((uint32_t *)ieeeAddress);

    pos = get_node_indfo_pos(id_l, &isFound);;

    if (-1 != pos) {
        nodes[pos].id_l[0] = id_l[0];
        nodes[pos].id_l[1] = id_l[1];
        nodes[pos].id_s = shortAddress;
        nodes[pos].isActive = 1;
        if (1 != isFound)
            ++num_nodes;
        return 0;
    }
    return 1;
 }

static uint32_t array_node_info_del(uint8_t *ieeeAddress)
{
    int i;
    uint32_t id_l[2];

    id_l[0] = *((uint32_t *)ieeeAddress+1);
    id_l[1] = *((uint32_t *)ieeeAddress);

    for (i = 0; i < num_nodes; i++)
        if (nodes[i].id_l[0] == id_l[0] && nodes[i].id_l[1] == id_l[1])
            break;

    if (i == num_nodes)
        return 1;

    for ( ; i < num_nodes-1; i++) {
        memcpy(nodes+i, nodes+i+1, sizeof(struct node_info));
    }
    --num_nodes;

    return 0;
}

struct node_info *array_node_info_get(uint32_t *idptr)
{
    int isFound = 0;
    int pos;
    pos = get_node_indfo_pos(idptr, &isFound);
    if (0 == isFound)
        return NULL;
    else
        return &nodes[pos];
}

struct node_info *array_node_info_saddr_get(uint16_t id)
{
    int pos = get_node_indfo_saddr_pos(id);
    if (pos < 0)
        return NULL;
    else
        return &nodes[pos];
}

struct node_info *array_node_info_at(uint16_t pos)
{
    if (pos < MAX_NODE_NUM)
        return &nodes[pos];
    else 
        NULL;
}

void nodeJoinHook(uint16_t shortAddress,uint8_t *ieeeAddress,uint8_t capability)
{
    printf("[mzspDemo]:node join 0x%04x \n",shortAddress);
    array_node_info_add(shortAddress, ieeeAddress);
}

void nodeLeaveHook(uint8_t *ieeeAddress,uint8_t rejoin)
{
    printf("[mzspDemo]:node leave 0x%08x%08x \n", \
            *((uint32_t *)ieeeAddress+1), \
            *((uint32_t *)ieeeAddress));
    array_node_info_del(ieeeAddress);
}

void zigbee_node_list(void)
{
    int i;
    uint8_t status[15];

    for (i = 0; i < num_nodes; i++) {
        zigbee_node_status_get(&nodes[i], status, sizeof(status), 0);
        printf("0x%08x%08x  0x%04x          0x%04x     0x%02x      %d.%d.%d.%d:%d   %s\n", 
                nodes[i].id_l[0], nodes[i].id_l[1], 
                nodes[i].id_s, nodes[i].profileId,
                nodes[i].endPoint, 
                nodes[i].majorVersion, nodes[i].minorVersion, 
                nodes[i].buildVersion, nodes[i].patchVersion,
                nodes[i].otaVersion,
                status);
    }
}

void zigbee_node_id_set(struct node_info* info, uint32_t* idptr)
{
    info->id_l[0] = idptr[0];
    info->id_l[1] = idptr[1];
}

void zigbee_node_ids_set(struct node_info* info, uint16_t id)
{
    info->id_s = id;
}

void zigbee_node_endPoint_set(struct node_info* info, uint8_t endPoint)
{
    info->endPoint = endPoint;
}

uint8_t zigbee_node_endPoint_get(struct node_info* info)
{
    return info->endPoint;
}

void zigbee_node_name_set(struct node_info* info, char* name)
{
    strncpy(&(info->name[0]), name, sizeof(info->name));
    info->name[sizeof(info->name)-1] = '\0';
}

char* zigbee_node_name_get(struct node_info* info)
{
    return &(info->name[0]);
}

void zigbee_node_modelname_setFix(struct node_info* info, char* name, uint8_t len)
{
    int lenOrigin;
    lenOrigin = strlen(info->modelname);
    if (lenOrigin == len && strncmp(info->modelname, name, len) == 0) {
        /*same model name*/
        printf("modelname KEPT\r\n");
    } else {
        /*new model name*/
        strncpy(&(info->modelname[0]), name, len + 1 < sizeof(info->modelname) ? len : sizeof(info->modelname));
        info->modelname[len + 1 < sizeof(info->modelname) ? len : sizeof(info->modelname)-1] = '\0';
    }
    printf("modelname is %s\r\n", info->modelname);
}

void zigbee_node_modelname_set(struct node_info* info, char* name)
{
    if (strcmp(info->modelname, name)) {
        strncpy(&(info->modelname[0]), name, sizeof(info->modelname));
        info->modelname[sizeof(info->modelname)-1] = '\0';
    }
    printf("modelname is %s\r\n", info->modelname);
}

char* zigbee_node_modelname_get(struct node_info* info)
{
    return &(info->modelname[0]);
}

void zigbee_node_version_firmware_set(struct node_info* info, uint32_t val)
{
    if (info->majorVersion != ((val >> 24) & 0XFF) ||
        info->minorVersion != ((val >> 16) & 0XFF) ||
        info->patchVersion != ((val >> 8) & 0XFF) ||
        info->otaVersion !=  (val & 0XFF)) {

        info->majorVersion = ((val >> 24) & 0XFF);
        info->minorVersion = ((val >> 16) & 0XFF);
        info->patchVersion = ((val >> 8) & 0XFF);
        info->otaVersion =  (val & 0XFF);
    }
}

uint8_t zigbee_node_versionMajor_get(struct node_info* info)
{
    return info->majorVersion;
}

uint8_t zigbee_node_versionMinor_get(struct node_info* info)
{
    return info->minorVersion;
}

uint8_t zigbee_node_versionPatch_get(struct node_info* info)
{
    return info->patchVersion;
}

uint8_t zigbee_node_versionBuild_get(struct node_info* info)
{
    return info->buildVersion;
}

uint32_t zigbee_node_version_firmware_get(struct node_info* info)
{
    uint32_t ver = 0;
    ver |= info->majorVersion;
    ver <<= 8;
    ver |= info->minorVersion;
    ver <<= 8;
    ver |= info->patchVersion;
    ver <<= 8;
    ver |= info->otaVersion;
    return ver;
}

void zigbee_node_version_build_set(struct node_info* info, uint8_t val)
{
    if (info->buildVersion != val) {
        info->buildVersion = val;
    }
}

void zigbee_node_onoff_set(struct node_info* info, uint8_t val)
{
    if (2 == val) {
        /*for toggle command*/
        info->onoff = (info->onoff ? 0 : 1);
    } else {
        /*normal onoff command*/
        if (info->onoff != val) {
            info->onoff = val;
        }
    }
}

uint8_t zigbee_node_onoff_get(struct node_info* info)
{
    return info->onoff;
}

#if 0
void zigbee_node_occFlag_set(struct node_info* info, uint8_t val)
{
    info->occFlag = val;
}

uint8_t zigbee_node_occFlag_get(struct node_info* info)
{
    return info->occFlag;
}

void zigbee_node_entryStatus_set(struct node_info* info, uint8_t val)
{
    info->entryStatus = val;
}

uint8_t zigbee_node_entryStatus_get(struct node_info* info)
{
    return info->entryStatus;
}
#endif

void zigbee_node_level_set(struct node_info* info, uint8_t val)
{
    info->level = val;
}

uint8_t zigbee_node_level_get(struct node_info* info)
{
    return info->level;
}

#if 0
void zigbee_node_mzspStatus_set(struct node_info* info, int val)
{
    info->mzspStatus = val;
}

int zigbee_node_mzspStatus_get(struct node_info* info)
{
    return info->mzspStatus;
}
#endif

void zigbee_node_cct_set(struct node_info* info, uint16_t val)
{
    info->cct = val;
}

uint16_t zigbee_node_colorX_get(struct node_info* info)
{
    return info->colorX;
}

uint16_t zigbee_node_colorY_get(struct node_info* info)
{
    return info->colorY;
}

uint8_t zigbee_node_colorHue_get(struct node_info* info)
{
    return info->colorHue;
}

uint8_t zigbee_node_colorSaturation_get(struct node_info* info)
{
    return info->colorSaturation;
}

uint16_t zigbee_node_cct_get(struct node_info* info)
{
    return info->cct;
}

static void _zigbee_node_colorRGB_set(struct node_info* info, uint8_t red, uint8_t green, uint8_t blue)
{
    printf("Set B RGB %u.%u.%u\r\n", info->colorRed, info->colorGreen, info->colorBlue);
    info->colorRed = red;
    info->colorGreen = green;
    info->colorBlue = blue;
    printf("Set A RGB %u.%u.%u\r\n", info->colorRed, info->colorGreen, info->colorBlue);
}

static void _zigbee_node_colorxy_set(struct node_info* info, uint16_t x, uint16_t y)
{
    info->colorX = x;
    info->colorY = y;
}

void zigbee_node_colorxy_set(struct node_info* info, uint16_t x, uint16_t y)
{
    _zigbee_node_colorxy_set(info, x, y);
}

void _zigbee_node_hue_set(struct node_info* info, uint8_t hue, uint8_t saturation)
{
    info->colorHue = hue;
    info->colorSaturation = saturation;
}

void zigbee_node_colorRGB_set(struct node_info* info, uint8_t red, uint8_t green, uint8_t blue)
{
    uint8_t hue, saturation, value;
    //zigbee_node_cscRgb2hsv(&hue, &saturation, &value, red, green, blue);
    _zigbee_node_colorRGB_set(info, red, green, blue);
    //_zigbee_node_hue_set(info, hue, saturation);
}

void zigbee_node_hue_set(struct node_info* info, uint8_t hue, uint8_t saturation)
{
    uint8_t red, green, blue;
    //zigbee_node_cscHsv2rgb(hue, saturation, 255, &red, &green, &blue);
    _zigbee_node_hue_set(info, hue, saturation);
    //_zigbee_node_colorRGB_set(info, red, green, blue);
}

uint8_t zigbee_node_colorRed_get(struct node_info* info)
{
    return info->colorRed;
}

uint8_t zigbee_node_colorGreen_get(struct node_info* info)
{
    return info->colorGreen;
}

uint8_t zigbee_node_colorBlue_get(struct node_info* info)
{
    return info->colorBlue;
}

uint8_t zigbee_node_colorWhite_get(struct node_info* info)
{
    return 255;
}

/*return value is RGBW*/
uint32_t zigbee_node_colorRGB_get(struct node_info* info)
{
    printf("Get RGB %u.%u.%u\r\n", info->colorRed, info->colorGreen, info->colorBlue);
    return (((uint32_t)info->colorRed) << 24) | (((uint32_t)info->colorGreen) << 16) | ((uint32_t)info->colorBlue << 8) | 0xFF;
}

#if 0
void zigbee_node_timeStampOnline_set(struct node_info* info, uint32_t val)
{
    info->timeStamp = val;
}

uint32_t zigbee_node_timeStampOnline_get(struct node_info* info)
{
    return info->timeStamp;
}

void zigbee_node_otaTimeStampOnline_set(struct node_info* info, uint32_t val)
{
    info->otaTimeStamp = val;
}

uint32_t zigbee_node_otaTimeStampOnline_get(struct node_info* info)
{
    return info->otaTimeStamp;
}
#endif

void zigbee_node_devtype_set(struct node_info* info, uint8_t val)
{
    if (info->devtype != val) {
        info->devtype = val;
    }
}

uint8_t zigbee_node_devtype_get(struct node_info* info)
{
    return info->devtype;
}

uint16_t zigbee_node_groupMember_get(struct node_info* info)
{
    return info->groupMember;
}

int zigbee_node_groupMember_set(struct node_info* info, uint16_t val)
{
    info->groupMember = val;
    return 0;
}

int zigbee_node_zones_is(struct node_info* info, uint8_t pos)
{
    /*one bit for one group*/
    if (pos < 8 * sizeof(info->groupMember)) {
        if (info->groupMember & (1 << pos)) {
            return 1;
        } else {
            return 0;
        }
    }
    return 0;
}

int zigbee_node_zones_add(struct node_info* info, uint8_t pos)
{
    /*one bit for one group*/
    if (pos < 8 * sizeof(info->groupMember)) {
        printf("Group membership set %4x\r\n", info->groupMember);
        if ((info->groupMember >> pos) & 0x01) {
            /*bit is already set*/
        } else {
            /*bit is newly set*/
            info->groupMember |= (1 << pos);
        }
        printf("Group membership done %4x\r\n", info->groupMember);
        return 0;
    }
    return -1;
}

int zigbee_node_zones_del(struct node_info* info, uint8_t pos)
{
    /*one bit for one group*/
    if (pos < 8 * sizeof(info->groupMember)) {
        printf("Group membership clear %4x\r\n", info->groupMember);
        info->groupMember &= (~(1 << pos));
        printf("Group membership done %4x\r\n", info->groupMember);
        return 0;
    }
    return -1;
}

int zigbee_node_zones_del_all(struct node_info* info)
{
    info->groupMember = 0;
    return 0;
}

int zigbee_node_zones_forEach(struct node_info* info, void (*callback)(void* param1, void* param2, struct node_info* info, uint8_t pos), void* param1, void* param2)
{
    int i;
    if (NULL == callback) {
        return -1;
    }
    for (i = 0; i < 8*sizeof(info->groupMember); i++) {
        if (info->groupMember & (1 << i)) {
            callback(param1, param2, info, i);
        }
    }
    return 0;
}

struct scene_info* zigbee_node_scenes_add(struct node_info* info, uint16_t sceneid)
{
#if 0
    int i;
    for (i = 0; i < info->num_scenes; i++) {
        if ( info->scenes[i].id == sceneid)
            break;
    }
    if (i == sizeof(info->scenes)/sizeof(info->scenes[0])) {
        return NULL;
    }
    if (i == info->num_scenes) {
        info->scenes[i].id = sceneid;
        (info->num_scenes)++;
    }
    return &(info->scenes[i]);
#endif
    return NULL;
}

struct scene_info* zigbee_node_scenes_get(struct node_info* info, uint16_t sceneid)
{
#if 0
    int i;
    for(i = 0; i < info->num_scenes; i++) {
        if ( info->scenes[i].id == sceneid)
            break;
    }
    return i == info->num_scenes ? NULL : &(info->scenes[i]);
#endif
    return NULL;
}

int zigbee_node_scenes_del(struct node_info* info, uint16_t sceneid)
{
#if 0
    int i, count;
    count = info->num_scenes;
    if (0 == count)
        return ERR_EMPTY_DATABASE;
    for (i = 0; i < count; i++) {
        if (info->scenes[i].id == sceneid )
            break;
    }
    if (i == count) {
        return ERR_NOT_FOUND;
    }
    memmove(&(info->scenes[i]), &(info->scenes[--(info->num_scenes)]), sizeof(struct scene_info));
#endif
    return 0;
}

int zigbee_node_scenes_name_set(struct scene_info* info, char* name)
{
#if 0
    strncpy(&(info->name[0]), name, sizeof(info->name));
    info->name[sizeof(info->name)-1] = '\0';
#endif
    return 0;
}


#if 0
int zigbee_node_str2id(uint8_t node_ieeeAddr[8], char* str)
{
    int i;
    for (i = 0 ; i < 8; i++){
        node_ieeeAddr[7-i] = hex2byte(str+2*i);
    }
    return 0;
}

int zigbee_node_id2str(char* hex, uint8_t* bin)
{
    int i, len = 8;

    for (i = 0; i < len; i++) {
        hex[2 * i] = num2hex(bin[7-i] >> 4);
        hex[(2 * i) + 1] = num2hex(bin[7-i] & 0x0f);
    }
    hex[2*i] = '\0';
    return 0;
}

uint16_t zigbee_node_get_clusterid(struct node_info* info)
{
    if (info->devtype == DOWN_LIGHT3 
        || info->devtype == DOWN_LIGHT2
        || info->devtype == DOWN_LIGHT1
        || info->devtype == EXT_COLOR_LIGHT
        || info->devtype == COLOR_LIGHT) {
        printf("light\r\n");
        return MZSP_ZCL_ONOFF_CLUSTER_ID;
    }
    if (info->devtype == ILLUM_SENSOR) {
        printf("illum\r\n");
        return MZSP_ZCL_ILL_MEASURE_SENSOR_CLUSTER_ID;
    }
    if (info->devtype == OCCUP_SENSOR) {
        printf("occ\r\n");
        return MZSP_ZCL_OCC_SENSOR_CLUSTER_ID;
    }
    printf("CRTITICAL ERROR: UNKNOW CLUSTER ID for devtype %x@%p\r\n",
        info->devtype, info);
    return 0;
}
#endif

int zigbee_node_isActive_set(struct node_info* info, unsigned int value)
{
    if (info->isActive != value) {
        info->isActive = value;
    }
    return 0;
}

unsigned int zigbee_node_isActive_get(struct node_info* info)
{
    return info->isActive;
}

int zigbee_node_version_get(struct node_info* info, char* buffer, int size)
{
    snprintf(buffer, size, "%u.%u.%u.%u_ota:%u",
        info->majorVersion,
        info->minorVersion,
        info->patchVersion,
        info->buildVersion,
        info->otaVersion);
    return 0;
}

#if 0
int zigbee_node_ota_init(struct node_info* info)
{
    info->otaEndpoint = 0;
    info->manufacturerId = 0;
    info->deviceType = 0;
    info->firmwareVersionCurrent = 0;
    zigbee_node_ota_offsetDownloading_set(info, 0);
    zigbee_node_ota_state_set(info, OTA_IDLE);
    return 0;
}

int zigbee_node_otaSize_set(struct node_info* info, uint32_t val)
{
    if (info->otaSize != val) {
        info->otaSize = val;
        zigbee_node_changedSet(info, ZIGBEE_NODE_INFO_CHANGED_OTA_OTASIZE);
    }
    return 0;
}

uint32_t zigbee_node_otaSize_get(struct node_info* info)
{
    return info->otaSize;
}

int zigbee_node_ota_state_set(struct node_info* info, uint16_t state)
{
    if (info->otaState != state) {
        info->otaState = state;
        zigbee_node_changedSet(info, ZIGBEE_NODE_INFO_CHANGED_OTA_OTASTATE);
    }
    return 0;
}

uint16_t zigbee_node_ota_state_get(struct node_info* info)
{
    return info->otaState;
}

int zigbee_node_ota_offsetDownloading_set(struct node_info* info, uint32_t size)
{
    if (info->offsetDowloading != size) {
        info->offsetDowloading = size;
        zigbee_node_changedSet(info, ZIGBEE_NODE_INFO_CHANGED_OTA_OTAPROGRESS);
    }
    return 0;
}

uint32_t zigbee_node_ota_offsetDownloading_get(struct node_info* info)
{
    return info->offsetDowloading;
}

int zigbee_node_ota_versionPresent_set(struct node_info* info, uint32_t version)
{
    info->firmwareVersionCurrent = version;
    return 0;
}

uint32_t zigbee_node_ota_versionPresent_get(struct node_info* info)
{
    return info->firmwareVersionCurrent;
}

int zigbee_node_ota_versionHardware_set(struct node_info* info, uint16_t version)
{
    info->hardwareVersion = version;
    return 0;
}

uint16_t zigbee_node_ota_versionHardware_get(struct node_info* info)
{
    return info->hardwareVersion;
}

int zigbee_node_ota_versionHardwarePresent_set(struct node_info* info, uint8_t version)
{
    info->hardwareVersionPresent = version;
    return 0;
}

uint8_t zigbee_node_ota_versionHardwarePresent_get(struct node_info* info)
{
    return info->hardwareVersionPresent;
}

int zigbee_node_ota_deviceType_set(struct node_info* info, uint16_t devtype)
{
    info->deviceType = devtype;
    return 0;
}

uint16_t zigbee_node_ota_deviceType_get(struct node_info* info)
{
    return info->deviceType;
}

int zigbee_node_ota_manufId_set(struct node_info* info, uint16_t manuid)
{
    info->manufacturerId = manuid;
    return 0;
}

uint16_t zigbee_node_ota_manufId_get(struct node_info* info)
{
    return info->manufacturerId;
}
#endif

int zigbee_node_profileId_set(struct node_info* info, uint16_t id)
{
    info->profileId = id;
    return 0;
}

uint16_t zigbee_node_profileId_get(struct node_info* info)
{
    return info->profileId;
}

char *zigbee_node_status_get(struct node_info* info, char* buffer, int size, int needOta)
{
    switch (zigbee_node_isActive_get(info)) {
        case 0:
            //XXX not a godd routine
            strncpy(buffer, "disconnected",
                sizeof("disconnected") > size ? size : sizeof("disconnected"));
            break;
        case 1:
            strncpy(buffer, "connecting",
                sizeof("connecting") > size ? size : sizeof("connecting"));
            break;
        case 2:
#if 0
            if (needOta) {
                switch (zigbee_node_ota_state_get(info)) {
                    case OTA_DOWNLOADING:
                        snprintf(buffer, size, "online(OTA %u/%u)",
                            zigbee_node_ota_offsetDownloading_get(info),
                            zigbee_node_otaSize_get(info));
                        break;
                    case OTA_WAIT:
                        strncpy(buffer, "online(OTA wait)",
                            sizeof("online, OTA wait") > size ? size : sizeof("online, OTA wait"));
                        break;
                    case OTA_ABORT:
                        strncpy(buffer, "online(OTA abort)",
                            sizeof("online, OTA abort") > size ? size : sizeof("online, OTA abort"));
                        break;
                    case OTA_QUEUE:
                        strncpy(buffer, "online(OTA queue)",
                            sizeof("online, OTA queue") > size ? size : sizeof("online, OTA queue"));
                        break;
                    default:
                        strncpy(buffer, "online",
                            sizeof("online") > size ? size : sizeof("online"));
                        break;
                }
            } else {
                strncpy(buffer, "online",
                    sizeof("online") > size ? size : sizeof("online"));
            }
#endif
            strncpy(buffer, "online",
                sizeof("online") > size ? size : sizeof("online"));
            break;
        default:
            strncpy(buffer, "unknown",
                sizeof("unknown") > size ? size : sizeof("unknown"));
            break;
    }
    buffer[size-1] = '\0';
    return buffer;
}

#if 0
int zigbee_node_versionZCGet(uint8_t *x, uint8_t *y, uint8_t *z, uint8_t *o, uint8_t *b)
{
    struct node_info* info = node_info_at(0);
    if (info) {
        *x = info->majorVersion;
        *y = info->minorVersion;
        *z = info->patchVersion;
        *b = info->otaVersion;
        *o = info->buildVersion;
        return 0;
    } else {
        return -1;
    }
}

int zigbee_node_isEmpty(struct node_info* info)
{
    if (ZIGBEE_NODE_INFO_ENTRYSTATUS_EMPTY == zigbee_node_entryStatus_get(info)) {
        return 1;
    } else {
        return 0;
    }
}
#endif
