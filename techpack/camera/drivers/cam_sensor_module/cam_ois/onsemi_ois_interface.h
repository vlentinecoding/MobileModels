/*
 * Copyright (c) 2021-2021, The Linux Foundation. All rights reserved.
 */
#ifndef _CAM_OIS_I2C_H_
#define _CAM_OIS_I2C_H_

#include <cam_sensor_cmn_header.h>

/* for I2C communication */
void RamWrite32A(struct cam_ois_ctrl_t *o_ctrl, uint32_t addr, uint32_t data, uint32_t delay);
void RamRead32A(struct cam_ois_ctrl_t *o_ctrl, uint32_t addr, uint32_t* data);
/* for I2C Multi Translation : Burst Mode*/
void CntWrt(struct cam_ois_ctrl_t *o_ctrl, uint8_t *data, uint32_t length, uint32_t delay) ;
void CntRd(struct cam_ois_ctrl_t *o_ctrl, uint32_t addr, uint8_t *data, uint32_t length) ;
void IOWrite32A(struct cam_ois_ctrl_t *o_ctrl, uint32_t addr, uint32_t data, uint32_t delay);
void IORead32A(struct cam_ois_ctrl_t *o_ctrl, uint32_t addr, uint32_t* data);

#endif /* _CAM_OIS_I2C_H_ */
