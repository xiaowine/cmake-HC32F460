/**
 *******************************************************************************
 * @file  hc32f4xx.h
 * @brief HC32F460 series adapter header for the DDL driver library.
 *******************************************************************************
 */
#ifndef __HC32F4XX_H__
#define __HC32F4XX_H__

#ifdef __cplusplus
extern "C" {
#endif

#if defined(HC32F460JCTA)
#include "hc32f460jcta.h"
#elif defined(HC32F460JETA)
#include "hc32f460jeta.h"
#elif defined(HC32F460JEUA)
#include "hc32f460jeua.h"
#elif defined(HC32F460KCTA)
#include "hc32f460kcta.h"
#elif defined(HC32F460KETA)
#include "hc32f460keta.h"
#elif defined(HC32F460KEUA)
#include "hc32f460keua.h"
#elif defined(HC32F460PCTB)
#include "hc32f460pctb.h"
#elif defined(HC32F460PEHB)
#include "hc32f460pehb.h"
#elif defined(HC32F460PETB)
#include "hc32f460petb.h"
#else
#error "Please define a concrete HC32F460 part, for example HC32F460KEUA."
#endif

#include "system_hc32f460.h"

#ifdef __cplusplus
}
#endif

#endif /* __HC32F4XX_H__ */
