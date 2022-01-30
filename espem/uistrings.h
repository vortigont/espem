#pragma once

// Set of flash-strings that might be reused multiple times within the code

// General
static const char C_ONE[] PROGMEM = "1";
static const char C_mkchart[] PROGMEM = "mkchart";
static const char C_js[] PROGMEM = "js";

//////////////////////
// Configuration variables names  - V_ prefix for 'Variable'
static const char V_EPOOLSIZE[] = "emplsz";             // sample pool size
static const char V_SMPL_PERIOD[] = "ems_prd";          // sampling period
static const char V_RX[] = "rx";             // rx pin
static const char V_TX[] = "tx";             // tx ping
static const char V_UART[] = "uart";         // uart interface
static const char V_EOFFSET[] = "eoffset";   // energy offset

// directly changed vars
static const char V_EPOLLENA[] PROGMEM = "dctlpoll";        // Enable/disable poller
static const char V_EPFFIX[] PROGMEM = "dctlpffix";         // PowerFactor value correction
static const char V_UI_UPDRT[] = "dctlupdrt";               // UI update rate
static const char V_ECOLLECTORSTATE[] = "dctlmtrx";	        // Metrics collector run/pause
static const char V_SMPLCNT[] = "dctlscnt";	                // Metrics graph - number of samples to draw in a small power chart


// UI blocks    - B_ prefix for 'web Block'
static const char B_ESPEM[] PROGMEM = "b_espem";
static const char B_ESPEMSET[] PROGMEM = "b_emset";
static const char B_WEATHER[] PROGMEM = "b_wthr";
static const char B_MORE[] PROGMEM = "b_more";

// direct control elements
static const char A_DIRECT_CTL[] PROGMEM = "dctl*";         // checkboxes/controls that should be processed in real-time


// UI handlers - A_ prefix for 'Action'
static const char A_SET_ESPEM[] PROGMEM = "a_setem";       //  ESPEM settings update 
static const char A_SET_UART[] PROGMEM =  "a_uart";
static const char A_SET_PZOPTS[] PROGMEM =  "a_pzopt";

// other constants
