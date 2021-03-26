#pragma once

// Set of flash-strings that might be reused multiple times within the code

// General
static const char T_HEADLINE[] PROGMEM = "ESP Энергометр";    // имя проекта
static const char C_ONE[] PROGMEM = "1";    // имя проекта

//////////////////////
// Configuration variables names  - V_ prefix for 'Variable'
static const char V_EPOOLSIZE[] = "emplsz";             // metrics collector mem pool size, KiB
// directly changed vars
static const char V_EPOLLENA[] PROGMEM = "dctlpoll";        // Enable/disable poller
static const char V_EPFFIX[] PROGMEM = "dctlpffix";         // PowerFactor value correction
static const char V_EPOLLRT[] = "dctlpllrt";            // Metrics collector pollrate
static const char V_ECOLLECTORSTATE[] = "dctlmtrx";	        // Metrics collector run/pause


// Matrix control
static const char V_MX_W[]  = "mx_w";	                // Matrix WIDTH (number of 8x8 MAX modules)
static const char V_MX_H[]  = "mx_h";	                // Matrix HIIGHT (number of 8x8 MAX modules)
static const char V_MX_VF[] = "mx_vf";	                // Canvas V-flip
static const char V_MX_HF[] = "mx_hf";	                // Canvas H-flip
static const char V_MX_OS[] = "mx_os";	                // Modules order - Serpentine
static const char V_MX_OV[] = "mx_ov";	                // Modules order - Vertical
static const char V_MX_MR[] = "mx_mr";	                // Module rotation


// UI blocks    - B_ prefix for 'web Block'
static const char B_ESPEM[] PROGMEM = "b_espem";
static const char B_ESPEMSET[] PROGMEM = "b_emset";
static const char B_WEATHER[] PROGMEM = "b_wthr";
static const char B_MORE[] PROGMEM = "b_more";

// direct control elements
static const char A_DIRECT_CTL[] PROGMEM = "dctl*";         // checkboxes/controls that should be processed in real-time


// UI handlers - A_ prefix for 'Action'
static const char A_UPD_WEATHER[] PROGMEM = "a_updw";       //  weather data refresh
static const char A_SET_WEATHER[] PROGMEM = "a_setwth";     //  weather settings set 
static const char A_SET_ESPEM[] PROGMEM = "a_setem";       //  Matrix settings set 


// other constants
