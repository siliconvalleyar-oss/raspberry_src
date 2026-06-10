///
/// @file hV_List_Constants.h
/// @brief Constants for Pervasive Displays Library Suite – Raspberry Pi port
///

#include <stdint.h>

#ifndef hV_LIST_CONSTANTS_RELEASE
#define hV_LIST_CONSTANTS_RELEASE 812

// Features
#define FEATURE_FAST            0x01
#define FEATURE_TOUCH           0x02
#define FEATURE_OTHER           0x04
#define FEATURE_WIDE_TEMPERATURE 0x08
#define FEATURE_RED             0x10
#define FEATURE_RED_YELLOW      0x20
#define FEATURE_BW              0x00
#define FEATURE_BWR             0x10
#define FEATURE_BWRY            0x20
#define FEATURE_HIGH_DEFINITION 0x40

// Update modes
#define UPDATE_NONE     0x00
#define UPDATE_GLOBAL   0x01
#define UPDATE_FAST     0x02
#define UPDATE_PARTIAL  0x03  // deprecated

// Screen families
#define FAMILY_SMALL    0x01
#define FAMILY_MEDIUM   0x02
#define FAMILY_LARGE    0x03

// Large-screen sub-panel selection
#define PANEL_CS_MASTER 0x01
#define PANEL_CS_SLAVE  0x02
#define PANEL_CS_BOTH   0x03

// Power scope
#define POWER_SCOPE_NONE        0x00
#define POWER_SCOPE_GPIO_ONLY   0x01
#define POWER_SCOPE_BUS_GPIO    0x11

// Power mode
#define POWER_MODE_AUTO   0x00
#define POWER_MODE_MANUAL 0x01

// FSM states
#define FSM_OFF         0x00
#define FSM_ON          0x11
#define FSM_SLEEP       0x10
#define FSM_GPIO_MASK   0x01
#define FSM_BUS_MASK    0x10

// Partial update state (deprecated)
#define PARTIAL_OFF     0x00
#define PARTIAL_ON      0x01
#define PARTIAL_READY   0x02

// Continuity
#define CONTINUITY_OFF   0x00
#define CONTINUITY_ON    0x01
#define CONTINUITY_READY 0x02

// Touch events
#define TOUCH_EVENT_NONE    0
#define TOUCH_EVENT_PRESS   1
#define TOUCH_EVENT_RELEASE 2
#define TOUCH_EVENT_MOVE    3

// Results
#define RESULT_SUCCESS  0
#define RESULT_ERROR    1

// Orientation
#define ORIENTATION_PORTRAIT    6
#define ORIENTATION_LANDSCAPE   7

#endif // hV_LIST_CONSTANTS_RELEASE
