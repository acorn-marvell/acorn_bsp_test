/*
 *  Copyright 2008-2013, Marvell International Ltd.
 *  All Rights Reserved.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "inc/common.h"
#include "inc/cli.h"
#include "inc/cli_utils.h"
#include "inc/zigbee_cli.h"
#include "inc/mzspAPI.h"
#include "inc/zigbee.h"

extern struct node_info *array_node_info_get(uint32_t *idptr);

/*
 * zigbee network commands
 */
static void cmd_network_list(int argc, char *argv[])
{
    /* get node list detail info*/
    printf("IEEE address        Short Address   ProfileId  EndPoint  Version     Status\n");
    /* call */
    zigbee_node_list();
}

static void cmd_network_update(int argc, char *argv[])
{
    uint32_t id_l[2];
    struct node_info *node;
    uint8_t epCount;
    uint8_t epList[16];
    uint16_t profileId;
    uint16_t deviceId;

    if (argc < 2) {
        printf("Please specify the IEEE address with \"0x\" prefix\n");
        return;
    }
    
    id_l[0] = id_l[1] = 0;
    a2hex64(argv[1], id_l);

    printf("node 0x%08x%08x updating...\n", id_l[0], id_l[1]);
    /* call */
    node = array_node_info_get(id_l);
    zigbee_node_ids_set(node, node->id_s);

    mzzdp_getActiveEndpoint(node->id_s, &epCount, (uint8_t **)epList); 
    zigbee_node_endPoint_set(node, epList[0]);

    mzzdp_getSimpleDesc(node->id_s, node->endPoint, &profileId, &deviceId);
    printf("device ID: 0x%04x\n", deviceId);
    zigbee_node_profileId_set(node, profileId);

    zigbee_node_isActive_set(node, 2);
}

static void cmd_network_permitjoin(int argc, char *argv[])
{
    uint32_t ret;
    uint8_t time;

    if (argc < 2) {
        printf("Please specify the permit join time with \"0x\" prefix\n");
        return;
    }

    time = a2hex(argv[1]);
    printf("permit join %d\n", time);

    /* call */
    ret = mznmc_setSelfPermitJoin(time);
    if (ret != 0) {
        printf("open zc permit join failed\n");
        return;
    }

    ret = mznmc_setMGMTPermitJoin(0xFFFC, time);
    if (ret != 0) {
        printf("open zr permit join failed\n");
        return;
    }
}

static void cmd_network_changechannel(int argc, char *argv[])
{
    uint32_t ret;
    int channel;

    if (argc == 1) {
        printf("Not Support!\n");
    }else if (argc == 2) {
        sscanf(argv[1], "%d", &channel);
        printf("set channel %d\n", channel);
        mznwc_MGMTNetworkUpdate(channel); 
    }
}

/*
 * zigbee zcl commands
 */
static void cmd_zcl_node_identify(int argc, char *argv[])
{
    uint32_t ret;
    int time;
    uint16_t id_s;
    
    if (argc < 3) {
        printf("Please specify the node short address and identify time\n");
        return;
    }
    
    id_s = a2hex(argv[1]);
    sscanf(argv[2], "%d", &time);

    printf("set node 0x%04x identify %d\n", id_s, time);

    /* call */
    ret = mzzcl_SetAPSHeaderParameters(0x0104, 1, 3, 2, id_s, 2);
    if (ret != 0) {
        printf("set aps parameter failed\n");
        return;
    }

    mzzcl_nodeIdentify(time);
}

#define ON     1
#define OFF    0
#define TOGGLE 2
static const char* on_off_status(uint8_t state)
{
    switch (state) {
    case ON:
        return "on";
    case OFF:
        return "off";
    case TOGGLE:
        return "toggle";
    default:
        return NULL;
    }
}

static void cmd_zcl_node_onoff(int argc, char *argv[])
{
    uint32_t ret;
    uint8_t state;
    uint16_t id_s;

    uint8_t dataType;
    uint8_t content[64];

    if (argc < 2) {
        printf("Usage: %s <short address> <on/off/toggle>\n\t1 - on\n\t0 - off\n\t2 - toggle\n",
                argv[0]);
        return;
    }

    if (argc == 2) {
        id_s = a2hex(argv[1]);
        ret = mzzcl_readZCLAttribute(0x0006, 0x0000, &dataType, (uint8_t**)&content);
        if (ret != 0) {
            printf("read attribute failed\n");
            return;
        }
        state = content[0];
        /* call */
        printf("onoff 0x%02x\n", state);
    } else if (argc == 3) {
        id_s = a2hex(argv[1]);
        sscanf(argv[2], "%d", &state);
        printf("set node %s\n", on_off_status(state));
        /* call */
        mzzcl_SetAPSHeaderParameters(0x0104, 1, 3, 2, id_s, 2);
        mzzcl_switchControl(state);
    }
}

static void cmd_zcl_zone_onoff(int argc, char *argv[])
{    
    uint8_t state;
    uint16_t id_g;

    if (argc < 3) {
        printf("Usage: %s <group id> <on/off/toggle>\n\t1 - on\n\t0 - off\n\t2 - toggle\n",
                argv[0]);
        return;
    }

    id_g = a2hex(argv[1]);
    sscanf(argv[2], "%d", &state);
    printf("set group %s\n", on_off_status(state));
    /* call */
    mzzcl_SetAPSHeaderParameters(0x0104, 1, 2, 1, id_g, 2);
    mzzcl_switchControl(state);
}

static void cmd_zcl_node_level(int argc, char *argv[])
{    
    uint32_t ret;
    int level;
    uint16_t id_s;

    uint8_t dataType;
    uint8_t content[64];

    if (argc < 2) {
        printf("Usage: %s <short address> <level>\n",
                argv[0]);
        return;
    }

    if (argc == 2) {
        id_s = a2hex(argv[1]);
        /* call */
        ret = mzzcl_readZCLAttribute(0x0008, 0x0000, &dataType, (uint8_t**)&content);
        level = content[0];
        if (ret != 0) {
            printf("read attribute failed\n");
            return;
        }
        printf("level 0x%02x\n", level);
    } else if (argc == 3) {
        id_s = a2hex(argv[1]);
        level = a2hex(argv[2]);
        printf("set node %d level 0x%02x\n", id_s, level);
        /* call */
        mzzcl_SetAPSHeaderParameters(0x0104, 1, 3, 2, id_s, 2);
        mzzcl_levelControl(4, level, 1);
    }
}

static void cmd_zcl_zone_level(int argc, char *argv[])
{
    int level;
    uint16_t id_g;;

    if (argc < 3) {
        printf("Usage: %s <group id> <level>\n",
                argv[0]);
        return;
    }

    id_g = a2hex(argv[1]);
    level = a2hex(argv[2]);
    /* call */
    printf("set zone 0x%02x level 0x%02x\n", id_g, level);
    mzzcl_SetAPSHeaderParameters(0x0104, 1, 2, 1, id_g, 2);
    mzzcl_levelControl(4, level, 1);
}

static void cmd_zcl_node_colorxy(int argc, char *argv[])
{
    uint16_t x, y;
    uint16_t id_s;

    uint8_t dataType;
    uint8_t content[64];

    if (argc < 2) {
        printf("Usage: %s <short address> <X> <Y>\n",
                argv[0]);
        return;
    }

    if (argc == 2) {
        id_s = a2hex(argv[1]);
        /* call */
        mzzcl_readZCLAttribute(0x0300, 0x0003, &dataType, (uint8_t**)&content);
        x = *((uint16_t *)content);
        mzzcl_readZCLAttribute(0x0300, 0x0004, &dataType, (uint8_t**)&content);
        y = *((uint16_t *)content);
        printf("color X:0x%02x Y:0x%02x", x, y);
    } else if (argc == 4) {
        id_s = a2hex(argv[1]);
        x = a2hex(argv[2]);
        y = a2hex(argv[3]);
        printf("set node %d color X:0x%02x Y:0x%02x\n", id_s, x, y);
        /* call */
        mzzcl_SetAPSHeaderParameters(0x0104, 1, 3, 2, id_s, 2);
        mzzcl_colorXYControl(x, y, 100);
    }
}

static void cmd_zcl_zone_colorxy(int argc, char *argv[])
{
    uint16_t x, y;
    uint16_t id_g;

   if (argc < 4) {
        printf("Usage: %s <group id> <X> <Y>\n",
                argv[0]);
        return;
    }

    id_g = a2hex(argv[1]);
    x = a2hex(argv[2]);
    y = a2hex(argv[3]);
    /* call */
    printf("set zone 0x%04x color X:0x%04x Y:0x%04x\n", id_g, x, y);
    mzzcl_SetAPSHeaderParameters(0x0104, 1, 2, 1, id_g, 2);
    mzzcl_colorXYControl(x, y, 100);

}

static void cmd_zcl_node_colorcct(int argc, char *argv[])
{
    uint16_t cct;
    uint16_t id_s;

    uint8_t dataType;
    uint8_t content[64];

    if (argc < 2) {
        printf("Usage: %s <short address> <cct>\n",
                argv[0]);
        return;
    }

    if (argc == 2) {
        id_s = a2hex(argv[1]);
        cct = 0x1240;
        /* call */
        mzzcl_readZCLAttribute(0x0300, 0x0007, &dataType, (uint8_t **)&content);
        cct = *((uint16_t *)content);
        printf("color CCT:0x%02x\n", cct);
    } else if (argc == 3) {
        id_s = a2hex(argv[1]);
        cct = a2hex(argv[2]);
        printf("set node %d color CCT:0x%04x\n", id_s, cct);
        /* call */
        mzzcl_SetAPSHeaderParameters(0x0104, 1, 3, 2, id_s, 2);
        mzzcl_colorCTControl(cct, 100);
    }

}

static void cmd_zcl_zone_colorcct(int argc, char *argv[])
{
    uint16_t cct;
    uint16_t id_g;

   if (argc < 3) {
        printf("Usage: %s <group id> <cct>\n",
                argv[0]);
        return;
    }

    id_g = a2hex(argv[1]);
    cct = a2hex(argv[2]);
    /* call */
    printf("set zone 0x%04x color CCT:0x%04x\n", id_g, cct);
    /* call */
    mzzcl_SetAPSHeaderParameters(0x0104, 1, 2, 1, id_g, 2);
    mzzcl_colorCTControl(cct, 0);
}

static void cmd_zcl_node_colorhs(int argc, char *argv[])
{
    uint16_t hue, sat;
    uint16_t id_s;
    uint8_t dataType;
    uint8_t content[64];

    if (argc < 2) {
        printf("Usage: %s <short address> <hue> <saturation>\n",
                argv[0]);
        return;
    }

    if (argc == 2) {
        id_s = a2hex(argv[1]);
        mzzcl_readZCLAttribute(0x0300, 0x0000, &dataType, (uint8_t **)&content);
        hue = *((uint16_t *)content);
        mzzcl_readZCLAttribute(0x0300, 0x0001, &dataType, (uint8_t **)&content);
        sat = *((uint16_t *)content);
        /* call */
        printf("color Hue:0x%04x Saturation:0x%04x\n", hue, sat);
    } else if (argc == 4) {
        id_s = a2hex(argv[1]);
        hue = a2hex(argv[2]);
        sat = a2hex(argv[3]);
        printf("set node %d color Hue:0x%02x Saturation:0x%04x\n", id_s, hue, sat);
        /* call */
        mzzcl_SetAPSHeaderParameters(0x0104, 1, 3, 2, id_s, 2);
        mzzcl_colorHueControl(hue, sat, 100);
    }

}

static void cmd_zcl_zone_colorhs(int argc, char *argv[])
{
    uint16_t hue, sat;
    uint16_t id_g;

   if (argc < 2) {
        printf("Usage: %s <group id> <Hue> <Saturation>\n",
                argv[0]);
        return;
    }

    id_g = a2hex(argv[1]);
    hue = a2hex(argv[2]);
    sat = a2hex(argv[3]);
    /* call */
    printf("set zone 0x%04x Hue:0x%04x Saturation:0x%04x\n", id_g, hue, sat);
    mzzcl_SetAPSHeaderParameters(0x0104, 1, 2, 1, id_g, 2);
    mzzcl_colorHueControl(hue, sat, 100);
}

static void cmd_zcl_node_scene_get(int argc, char *argv[])
{
    uint16_t id_s, id_g;

   if (argc < 3) {
        printf("Usage: %s <short address> <group id>\n",
                argv[0]);
        return;
    }

    id_s = a2hex(argv[1]);
    id_g = a2hex(argv[2]);
    /* call */
    printf("scene: 0x%04x %d/16 \n", id_g, 0);
    printf("Not Support!\n");

}

#if 0
static void cmd_zcl_zone_scene_get(int argc, char *argv[])
{
}
#endif

static void cmd_zcl_node_scene_recall(int argc, char *argv[])
{
    uint16_t id_s, id_g;
    uint8_t scene;

    if (argc < 4) {
        printf("Usage: %s <short address> <group id> <scene>\n",
                argv[0]);
        return;
    }

    id_s = a2hex(argv[1]);
    id_g = a2hex(argv[2]);
    scene = a2hex(argv[3]);

    /* call */
    printf("call node 0x%04x with scene 0x%02x\n", id_s, scene);
    printf("Not Support!\n");
}

static void cmd_zcl_node_scene_delall(int argc, char *argv[])
{
    uint16_t id_s, id_g;

    if (argc < 3) {
        printf("Usage: %s <short address> <group id>\n",
                argv[0]);
        return;
    }

    id_s = a2hex(argv[1]);
    id_g = a2hex(argv[2]);

    /* call */
    printf("del node 0x%04x all scene", id_s);
    printf("Not Support!\n");
}

static void cmd_zcl_node_scene_store(int argc, char *argv[])
{ 
    uint16_t id_s, id_g;
    uint8_t scene;

    if (argc < 4) {
        printf("Usage: %s <short address> <group id> <scene>\n",
                argv[0]);
        return;
    }

    id_s = a2hex(argv[1]);
    id_g = a2hex(argv[2]);
    scene = a2hex(argv[3]);
    
    /* call */
    printf("store node 0x%04x scene 0x%02x\n", id_s, scene);
    printf("Not Support!\n");
}

static void cmd_zcl_node_scene_del(int argc, char *argv[])
{
    uint16_t id_s, id_g;
    uint8_t scene;

    if (argc < 4) {
        printf("Usage: %s <short address> <group id> <scene>\n",
                argv[0]);
        return;
    }

    id_s = a2hex(argv[1]);
    id_g = a2hex(argv[2]);
    scene = a2hex(argv[3]);
    
    /* call */
    printf("del node 0x%04x scene 0x%02x\n", id_s, scene);

}

static void cmd_zcl_zone_scene_recall(int argc, char *argv[])
{
    uint16_t id_g;
    uint8_t scene;

    if (argc < 3) {
        printf("Usage: %s <group id> <scene>\n",
                argv[0]);
        return;
    }

    id_g = a2hex(argv[1]);
    scene = a2hex(argv[2]);

    /* call */
    printf("call zone 0x%04x with scene 0x%02x\n", id_g, scene);
    mzzcl_SetAPSHeaderParameters(0x0104, 1, 2, 1, id_g, 2);
    mzzcl_SceneControl(0x05, id_g, scene); 
}

static void cmd_zcl_zone_scene_delall(int argc, char *argv[])
{
    uint16_t id_g;

    if (argc < 2) {
        printf("Usage: %s <group id>\n",
                argv[0]);
        return;
    }

    id_g = a2hex(argv[1]);

    /* call */
    printf("del zone 0x%04x all scene\n", id_g);
    printf("Not Support!\n");
}

static void cmd_zcl_zone_scene_del(int argc, char *argv[])
{
    uint16_t id_g;
    uint8_t scene;

    if (argc < 3) {
        printf("Usage: %s <group id> <scene>\n",
                argv[0]);
        return;
    }

    id_g = a2hex(argv[1]);
    scene = a2hex(argv[2]);
    
    /* call */
    printf("del zone 0x%04x scene 0x%02x\n", id_g, scene);
    mzzcl_SetAPSHeaderParameters(0x0104, 1, 2, 1, id_g, 2);
    mzzcl_SceneControl(0x02, id_g, scene); 
}

static void cmd_zcl_zone_scene_store(int argc, char *argv[])
{ 
    uint16_t id_g;
    uint8_t scene;

    if (argc < 3) {
        printf("Usage: %s <group id> <scene>\n",
                argv[0]);
        return;
    }

    id_g = a2hex(argv[1]);
    scene = a2hex(argv[2]);
    
    /* call */
    printf("store zone 0x%04x scene 0x%02x\n", id_g, scene);
    mzzcl_SetAPSHeaderParameters(0x0104, 1, 2, 1, id_g, 2);
    mzzcl_SceneControl(0x04, id_g, scene); 
}

static void cmd_zcl_zone_get(int argc, char *argv[])
{
    int i;
    uint16_t id_s;
    uint8_t cap, groupCount;
    uint16_t groupList[16];


   if (argc < 2) {
        printf("Usage: %s <node short address>\n",
                argv[0]);
        return;
    }

    id_s = a2hex(argv[1]);
    /* call */
    mzzcl_SetAPSHeaderParameters(0x0104, 1, 3, 2, id_s, 2);
    mzzcl_getGroupMembership(&cap, &groupCount, (uint16_t**)&groupList);
    printf("zone num: %d/%d\n", groupCount, groupCount+cap);
    for (i = 0; i < groupCount; i++)
        printf("zone id 0x%04x\n", groupList[i]);
}

static void cmd_zcl_zone_add(int argc, char *argv[])
{
    uint16_t id_s, id_g;

   if (argc < 3) {
        printf("Usage: %s <short address> <group id>\n",
                argv[0]);
        return;
    }

    id_s = a2hex(argv[1]);
    id_g = a2hex(argv[2]);
    /* call */
    printf("add node 0x%04x to zone 0x%04x \n", id_s, id_g);
    mzzcl_SetAPSHeaderParameters(0x0104, 1, 3, 2, id_s, 2);
    mzzcl_addGroup(id_g, "\0");

}

static void cmd_zcl_zone_remove(int argc, char *argv[])
{
    uint16_t id_s, id_g;

    if (argc < 2) {
        printf("Usage: %s <short address> <group id> or <short address>\n",
                argv[0]);
        return;
    }

    if (argc == 2) {
        id_s = a2hex(argv[1]);
        /* call */
        printf("del node 0x%04x from all groups\n", id_s);
        printf("Not support!\n");
        //mzzcl_SetAPSHeaderParameters(0x0104, 1, 3, 2, id_s, 2);
    } else if (argc == 3) {
        id_s = a2hex(argv[1]);
        id_g = a2hex(argv[2]);
        /* call */
        printf("del node 0x%04x from group 0x%04x\n", id_s, id_g);
        mzzcl_SetAPSHeaderParameters(0x0104, 1, 3, 2, id_s, 2);
        mzzcl_removeGroup(id_g);
    }
}

/*
 * zigbee zdo commands
 */
static void cmd_zdo_leave(int argc, char *argv[])
{
#if 0
    uint32_t id_l[2];

    if (argc < 2) {
        printf("Please specify the ieee address with \"0x\" prefix\n");
        return;
    }
    
    id_l[0] = id_l[1] = 0;
    a2hex64(argv[1], id_l);

    printf("node 0x%08x%08x leaving...\n", id_l[0], id_l[1]);
#endif
    uint16_t id_s;

    if (argc < 2) {
        printf("Please specify the short address with \"0x\" prefix\n");
        return;
    }

    id_s = a2hex(argv[1]);
    printf("node 0x%08x leaving...\n", id_s);
    /* call */
    mznwc_MGMTNodeLeave(id_s, 0);
}

static struct cli_command zigbee_cmds[] = {
    {"network-list", NULL, cmd_network_list}, 
    {"network-update", "<IEEE address>", cmd_network_update},
    {"network-permitjoin", "<time secs>", cmd_network_permitjoin},
    {"network-channel", "<channel(11-26)>", cmd_network_changechannel},
    {"zcl-node-identify", "<node> <time secs>", cmd_zcl_node_identify},
    {"zcl-node-onoff", "<node> <on/off/toggle>", cmd_zcl_node_onoff},
    {"zcl-zone-onoff", "<zone> <on/off/toggle>", cmd_zcl_zone_onoff},
    {"zcl-node-level", "<node> <level>", cmd_zcl_node_level},
    {"zcl-zone-level", "<zone> <level>", cmd_zcl_zone_level},
    {"zcl-node-colorxy", "<node> <x> <y>", cmd_zcl_node_colorxy},
    {"zcl-zone-colorxy", "<zone> <x> <y>", cmd_zcl_zone_colorxy},
    {"zcl-node-colorcct", "<node> <cct>", cmd_zcl_node_colorcct},
    {"zcl-zone-colorcct", "<zone> <cct>", cmd_zcl_zone_colorcct},
    {"zcl-node-colorhs", "<node> <hue> <saturation>", cmd_zcl_node_colorhs},
    {"zcl-zone-colorhs", "<zone> <hue> <saturation>", cmd_zcl_zone_colorhs},
    //{"zcl-node-scene-get", "<node> <zone>", cmd_zcl_node_scene_get},
    /*{"zcl-zone-scene-get", "<node> <zone>", cmd_zcl_zone_scene_get},*/
    //{"zcl-node-scene-recall", "<node> <zone> <scene>", cmd_zcl_node_scene_recall},
    {"zcl-zone-scene-recall", "<zone> <scene>", cmd_zcl_zone_scene_recall},
    //{"zcl-node-scene-delall", "<node> <zone>", cmd_zcl_node_scene_delall},
    //{"zcl-zone-scene-delall", "<zone>", cmd_zcl_zone_scene_delall},
    //{"zcl-node-scene-del", "<node> <zone> <scene>", cmd_zcl_node_scene_del},
    {"zcl-zone-scene-del", "<zone> <scene>", cmd_zcl_zone_scene_del},
    //{"zcl-node-scene-store", "<node> <zone> <scene>", cmd_zcl_node_scene_store},
    {"zcl-zone-scene-store", "<zone> <scene>", cmd_zcl_zone_scene_store},
    {"zcl-zone-get", "<node>", cmd_zcl_zone_get},
    {"zcl-zone-add", "<node> <zone>", cmd_zcl_zone_add},
    {"zcl-zone-remove", "<node> <zone>", cmd_zcl_zone_remove},
    {"zdo-leave", "<short address>", cmd_zdo_leave},
};

void zigbee_cli_init(void)
{
    int i;

    for (i = 0; i < sizeof(zigbee_cmds) / sizeof(struct cli_command); i++)
        if (cli_register_command(&zigbee_cmds[i]))
            printf("zibgee command register error\n");
}

void zigbee_cli_deinit(void)
{
    int i;

    for (i = 0; i < sizeof(zigbee_cmds) / sizeof(struct cli_command); i++)
        if (cli_unregister_command(&zigbee_cmds[i]))
            printf("zibgee command unregister error\n");
}


