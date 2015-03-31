#ifndef __ZIGBEE_H__
#define __ZIGBEE_H__

struct node_info {
     uint32_t id_l[2];
    uint16_t id_s;
    uint8_t onoff;
    uint8_t level;
    uint8_t colorRed;
    uint8_t colorGreen;
    uint8_t colorBlue;
    uint8_t colorW;
    uint16_t cct;
    uint16_t colorX;
    uint16_t colorY;
    uint16_t profileId;
    uint8_t colorHue;
    uint8_t colorSaturation;
    uint16_t devtype;
 
    uint8_t endPoint;
    uint8_t majorVersion;
    uint8_t minorVersion;
    uint8_t patchVersion;
    uint8_t buildVersion;
    uint8_t otaVersion;

    /*0: offline, 1: connecting, 2: online*/
    unsigned int isActive:2; 
    /*fileds for OTA*/
    uint8_t otaEndpoint;
    uint16_t manufacturerId;
    uint16_t deviceType;
    uint8_t hardwareVersionPresent;
    uint16_t hardwareVersion;
    uint32_t firmwareVersionCurrent;
    uint32_t firmwareVersionDowloading;
    uint32_t otaSize;
    uint16_t otaState;
 
    char name[16];
    char modelname[33];
    /*zone filed*/
    uint16_t groupMember; // need discuss 
};

struct scene_info {
    uint8_t id;
    char name[16];
    uint16_t groupID;
};

struct zone_info {
    uint16_t id;
    char name[16];
    uint8_t sceneCap;
};


void nodeJoinHook(uint16_t shortAddress,uint8_t * ieeeAddress,uint8_t capability);
void nodeLeaveHook(uint8_t *ieeeAddress,uint8_t rejoin);
void zigbee_node_list(void);
void zigbee_node_endPoint_set(struct node_info* info, uint8_t endPoint);
int zigbee_node_profileId_set(struct node_info* info, uint16_t id);
int zigbee_node_isActive_set(struct node_info* info, unsigned int value);
char *zigbee_node_status_get(struct node_info* info, char* buffer, int size, int needOta);

#endif


