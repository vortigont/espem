/*
LOG macro will enable/disable logs to serial depending on ESPEM_DEBUG build-time flag
*/
#pragma once

#ifndef ESPEM_DEBUG_PORT
#define ESPEM_DEBUG_PORT Serial
#endif

#ifndef ESPEM_DEBUG_LEVEL
#define ESPEM_DEBUG_LEVEL 2
#endif

// undef possible LOG macros
#ifdef LOG
  #undef LOG
#endif
#ifdef LOGV
  #undef LOGV
#endif
#ifdef LOGD
  #undef LOGD
#endif
#ifdef LOGI
  #undef LOGI
#endif
#ifdef LOGW
  #undef LOGW
#endif
#ifdef LOGE
  #undef LOGE
#endif

static constexpr const char* S_V = "V: ";
static constexpr const char* S_D = "D: ";
static constexpr const char* S_I = "I: ";
static constexpr const char* S_W = "W: ";
static constexpr const char* S_E = "E: ";

#if defined(ESPEM_DEBUG_LEVEL) && ESPEM_DEBUG_LEVEL == 5
	#define LOGV(tag, func, ...) ESPEM_DEBUG_PORT.print(S_V); ESPEM_DEBUG_PORT.print(tag); ESPEM_DEBUG_PORT.print((char)0x9); ESPEM_DEBUG_PORT.func(__VA_ARGS__)
#else
	#define LOGV(...)
#endif

#if defined(ESPEM_DEBUG_LEVEL) && ESPEM_DEBUG_LEVEL > 3
	#define LOGD(tag, func, ...) ESPEM_DEBUG_PORT.print(S_D); ESPEM_DEBUG_PORT.print(tag); ESPEM_DEBUG_PORT.print((char)0x9); ESPEM_DEBUG_PORT.func(__VA_ARGS__)
#else
	#define LOGD(...)
#endif

#if defined(ESPEM_DEBUG_LEVEL) && ESPEM_DEBUG_LEVEL > 2
	#define LOGI(tag, func, ...) ESPEM_DEBUG_PORT.print(S_I); ESPEM_DEBUG_PORT.print(tag); ESPEM_DEBUG_PORT.print((char)0x9); ESPEM_DEBUG_PORT.func(__VA_ARGS__)
	// compat macro
	#define LOG(func, ...) ESPEM_DEBUG_PORT.func(__VA_ARGS__)
#else
	#define LOGI(...)
	// compat macro
	#define LOG(...)
#endif

#if defined(ESPEM_DEBUG_LEVEL) && ESPEM_DEBUG_LEVEL > 1
	#define LOGW(tag, func, ...) ESPEM_DEBUG_PORT.print(S_W); ESPEM_DEBUG_PORT.print(tag); ESPEM_DEBUG_PORT.print((char)0x9); ESPEM_DEBUG_PORT.func(__VA_ARGS__)
#else
	#define LOGW(...)
#endif

#if defined(ESPEM_DEBUG_LEVEL) && ESPEM_DEBUG_LEVEL > 0
	#define LOGE(tag, func, ...) ESPEM_DEBUG_PORT.print(S_E); ESPEM_DEBUG_PORT.print(tag); ESPEM_DEBUG_PORT.print((char)0x9); ESPEM_DEBUG_PORT.func(__VA_ARGS__)
#else
	#define LOGE(...)
#endif


// LOG tags
static constexpr const char* T_Effect = "Effect";
static constexpr const char* T_EffCfg = "EffCfg";
static constexpr const char* T_EffWrkr = "EffWrkr";
static constexpr const char* T_Fade = "Fade";
static constexpr const char* T_Module = "Module";
static constexpr const char* T_WebUI = "WebUI";
static constexpr const char* T_ModMGR = "ModMGR";
