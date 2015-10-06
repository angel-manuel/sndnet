/**
 * @file
 * Contains SecondNet message types
 * */

#ifndef SN_MSG_TYPE_H_
#define SN_MSG_TYPE_H_

#include "router.h"

#include <stdint.h>

/**
 * SecondNet message types type
 * */
typedef uint16_t sn_msg_type_t;

/**
 * User message, that is, a message of a higher layer.
 * */
#define SN_MSG_TYPE_USER 0

typedef struct {
    unsigned char payload[0]; /**< User payload */
} sn_msg_type_user_t;

/**
 * Query for table entries
 * */
#define SN_MSG_TYPE_QUERY_TABLE 1

typedef struct {
    uint16_t query_id; /**< Query ID: Result will have the same ID */
    uint16_t min_level; /**< Minimum level */
    uint16_t max_level; /**< Maximum level */
} sn_msg_type_query_table_t;

/**
 * Query for leafset entries
 * */
#define SN_MSG_TYPE_QUERY_LEAFSET 2

typedef struct {
    uint16_t query_id; /**< Query ID: Result will have the same ID */
    int32_t min_position; /**< Minimum postion */
    int32_t max_position; /**< Maximum position */
} sn_msg_type_query_leafset_t;

/**
 * Query results
 * */
#define SN_MSG_TYPE_QUERY_RESULT 3

typedef struct {
    uint16_t query_id; /**< Query ID: Same as that of the request */
    sn_router_query_ser_t result; /**< Query result */
} sn_msg_type_query_result_t;

#endif/*SN_MSG_TYPE_H_*/
