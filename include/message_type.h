#ifndef SNDNET_MESSAGE_TYPE_H_
#define SNDNET_MESSAGE_TYPE_H_

#include <stdint.h>

/**
 * SecondNet message types type
 * */
typedef uint16_t sndnet_message_type_t;

/**
 * User message, that is, a message of a higher layer.
 * */
#define SNDNET_MSG_TYPE_USER 0

#endif/*SNDNET_MESSAGE_TYPE_H_*/