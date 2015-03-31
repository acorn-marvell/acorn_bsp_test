/*****************************************************************************//**
 * @file     mzspAPI.h
 * @brief    mzsp API 4 Linux header file
 * @version  V0.0.01
 * @date     22. July 2014
 * @author   Embedded software Team
 *
 * @note
 * Copyright (C) 2011 Marvell Limited. All rights reserved.
 *
 * @par
 * Marvell Limited is supplying this software which provides customers with
 * programming information regarding the products.Marvell has no responsibility
 * or liability for the use of the software.Marvell not guarantee the correctness
 * of this software.Marvell reserves the right to make changes in the software
 * without notification.
 *
 ********************************************************************************
 * CHANGE HISTORY
 *
 *  dd/mmm/yy     Code Ver      Author     Description
 *
 * 
 *
 *******************************************************************************/
#ifndef MZSP_API_H
#define MZAP_API_H

#define DEBUG_PRINTF
#ifdef DEBUG_PRINTF 
//#define debug_printf(...) fprintf (stderr, __VA_ARGS__) 
#define debug_printf(format, ...) fprintf (stderr, format, ## __VA_ARGS__)
#else 
#define debug_printf(...) do{}while(0); 
#endif 

#define MZ_NORMAL_URAT 0
#define MZ_USB_URAT 1

/** @defgroup Host_API_Public_Function_Declaration API_Public_Function_Declaration
 *  @brief API functions statement
 *  @{
 */
typedef void (* joinIndicateHook)(uint16_t shortAddress,uint8_t * ieeeAddress,uint8_t capability);

typedef void (* leaveIndicateHook)(uint8_t * ieeeAddress,uint8_t rejoin);

typedef void (* rptIndicateHook)(uint8_t srcEndpoint,uint8_t srcAddrMode,uint8_t * ieeeAddress,uint16_t attrID,uint8_t attrType,uint8_t * payload);

void mzgeneral_setJoinIndFunc(joinIndicateHook joinIndFunc);

void mzgeneral_setLeaveIndFunc(leaveIndicateHook leaveIndFunc);

uint32_t mzgeneral_init(uint8_t comPort, uint8_t mode);

uint32_t mzzcl_SetAPSHeaderParameters(uint16_t profileID, uint8_t srcEndPoint,uint8_t destEndPoint,uint8_t destAddrMode,uint16_t destAddr,uint8_t txOptions);

uint32_t mzzcl_readZCLAttribute(uint16_t clusterID,uint16_t attrID, uint8_t * returnDataType, uint8_t ** returnDataContent);

uint32_t mzzcl_writeZCLAttribute(uint16_t clusterID,uint8_t writeType,uint16_t mfcode,uint16_t attrID,uint8_t attrType, uint8_t dataLen,uint8_t * dataArray);

uint32_t mzzcl_ReportAttributeConfig(uint16_t clusterID,uint16_t attrID,uint8_t attrType, 
                                          uint16_t minInterval,uint16_t maxInterval, uint8_t * rptChange);

uint32_t mzzcl_switchControl(uint8_t switchFlag);

uint32_t mzzcl_levelControl(uint8_t controlMode, uint8_t level,uint16_t transitionTime);

uint32_t mzzcl_colorHueControl(uint8_t hue,uint8_t saturation,uint16_t transitionTime);

uint32_t mzzcl_colorXYControl(uint16_t color_X,uint16_t color_Y,uint16_t transitionTime);

uint32_t mzzcl_colorCTControl(uint16_t CT,uint16_t transitionTime);

uint32_t mzzcl_colorLoop(uint16_t loopTime);

uint32_t mzzcl_addGroup(uint16_t groupID,char * groupName);

uint32_t mzzcl_removeGroup(uint16_t groupID);

uint32_t mzzcl_getGroupMembership(uint8_t * cap ,uint8_t * groupCount, uint16_t ** groupList);

uint32_t mzzcl_SceneControl(uint8_t controlMode,uint16_t groupID,uint8_t sceneID);//ControlMode -- Remove/Store/recall 

uint32_t mzzcl_nodeIdentify(uint16_t identifyTime);

uint32_t mznmc_setSelfPermitJoin(uint8_t permitJoinTime);

uint32_t mznmc_setMGMTPermitJoin(uint16_t destAddr,uint8_t permitJoinTime);

uint32_t mznwc_MGMTNetworkUpdate(uint8_t channel); 

uint32_t mznwc_MGMTNodeLeave(uint16_t destAddr, uint8_t rejoinAndRemoveChildrenFlag);

uint32_t mztl_touchLinkScan(uint8_t actionType, uint8_t *listSize,uint8_t ** nodeIeeeAddrList);

uint32_t mztl_touchLinkSelectTarget(uint8_t index,uint16_t * shortAddr);

uint32_t mzzdp_getNetworkAddress(uint8_t * ieeeAddress, uint16_t * returnShortAddress);

uint32_t mzzdp_getIeeeAddress(uint16_t shortAddress, uint16_t ** returnIeeeAddress);

uint32_t mzzdp_getActiveEndpoint(uint16_t shortAddr,uint8_t * returnEndPointCount, uint8_t ** returnEndPointList); 

uint32_t mzzdp_getSimpleDesc(uint16_t shortAddr,uint8_t endPoint,uint16_t * returnProfileID, uint16_t * returnDeviceID);

uint32_t mzzdp_bind(uint16_t clusterID,uint8_t * srcIeeeAddr,uint8_t srcEndPoint, uint8_t destAddrmode,uint8_t * destIeeeAddr,uint8_t destEndPoint);

#if 0
uint32_t mzSpecial_getNeighborTableEntryNumber(uint8_t * returnEntriesNum);

uint32_t mzSpecial_setNeighborTableIndex(uint8_t index);

uint32_t mzSpecial_getNeighborTableEntry();

uint32_t mzSpecial_resetNeighborTableEntry();

uint32_t mzSpecial_getVersion(uint32_t * returnVersion);

uint32_t mzSpecial_sendMTOORouteReq();

uint32_t mzSpecial_setConcentratorDiscoveryTime(uint8_t time);

uint32_t mzSpecial_setGWCmdTimeoutReq(uint8_t zbProRspTimeout, uint8_t zbProCfmTimeout,uint8_t routeDiscoveryTimeout,uint8_t APSAckTimeout);

uint32_t mztl_setRSSIOffsetAndThreshold(uint8_t offset,uint8_t initiatorThreshold,uint8_t targetThreshold, uint8_t enableFilter);

uint32_t mztl_setTouchLinkScanPower(uint8_t txPower);

#endif

#endif
