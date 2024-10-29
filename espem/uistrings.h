#pragma once

// Set of flash-strings that might be reused multiple times within the code

// General
static constexpr const char* C_EspEM = "EspEM";
static constexpr const char* C_espem = "espem";
static constexpr const char* C_espem_ui = "espem.ui";
static constexpr const char* C_mkchart = "mkchart";
static constexpr const char* C_mkgauge = "mkgauge";
static constexpr const char* C_minichart = "minichart";
static constexpr const char* C_smpchart = "smpchart";
static constexpr const char* C_mqtt_pzem_jmetrics = "pub/pzem/jmetrics";
static constexpr const char* C_sample = "sample";
static constexpr const char* C_scnt = "scnt";                  // samle counter
static constexpr const char* C_tier = "tier";
static constexpr const char* C_lchart = "lchart";


//////////////////////
// Configuration variables names  - V_ prefix for 'Variable'
static constexpr const char* V_TS_T1_CNT = "t1cnt";            // default Tier 1 TimeSeries count
static constexpr const char* V_TS_T1_INT = "t1int";            // default Tier 1 TimeSeries interval
static constexpr const char* V_TS_T2_CNT = "t2cnt";            // default Tier 2 TimeSeries count
static constexpr const char* V_TS_T2_INT = "t2int";            // default Tier 2 TimeSeries interval
static constexpr const char* V_TS_T3_CNT = "t3cnt";            // default Tier 3 TimeSeries count
static constexpr const char* V_TS_T3_INT = "t3int";            // default Tier 3 TimeSeries interval
static constexpr const char* V_RX = "rx";                      // rx pin
static constexpr const char* V_TX = "tx";                      // tx ping
static constexpr const char* V_UART = "uart";                  // uart interface
static constexpr const char* V_EOFFSET = "eoffset";            // energy offset

// directly changed vars, must match actions with prefixed "dctl_"
static constexpr const char* V_EPOLLENA = "poll";              // Enable/disable poller
static constexpr const char* V_EPFFIX = "pffix";               // PowerFactor value correction
static constexpr const char* V_UI_UPDRT = "updaterate";        // UI update rate
static constexpr const char* V_ECOLLECTORSTATE = "collector";  // Metrics collector run/pause
static constexpr const char* V_SMPLCNT = "smplcnt";	        // Metrics graph - number of samples to draw in a small power chart


// UI blocks    - B_ prefix for 'web Block'
static constexpr const char* A_ui_page_espem = "ui_page_espem";
static constexpr const char* A_ui_page_espem_setup = "ui_page_espem_setup";
static constexpr const char* A_ui_page_dataexport = "ui_page_dataexport";

// direct control elements
static constexpr const char* A_DIRECT_CTL = "dctl_*";              // checkboxes/controls that should be processed onChange

// UI handlers - A_ prefix for 'Action'
static constexpr const char* A_set_espem_pool = "set_espem_pool";  //  ESPEM settings update 
static constexpr const char* A_SET_UART =  "set_uart";
static constexpr const char* A_SET_PZOPTS =  "set_nrgoffset";
static constexpr const char* A_SET_MCOLLECTOR = "set_mcollector";    // apply metrics collector settings

// onChange controls actions
static constexpr const char* A_EPOLLENA = "dctl_poll";             // Enable/disable poller
static constexpr const char* A_EPFFIX = "dctl_pffix";              // PowerFactor value correction
static constexpr const char* A_UI_UPDRT = "dctl_updaterate";       // UI update rate
static constexpr const char* A_ECOLLECTORSTATE = "dctl_collector"; // Metrics collector run/pause
static constexpr const char* A_SMPLCNT = "dctl_scnt";	            // Metrics graph - number of samples to draw in a small power chart
static constexpr const char* A_TS_TIER = "dctl_tier";              // drop-down selector for power chart TS id 
// other constants
