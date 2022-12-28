// localization resources goes here

#pragma once

/**
 *  Dictionary size
 *  must be more or equal to the number of messages in TD Enum
 */
#define UI_DICT_SIZE 20

/**
 * Text-Dictionary Enums for language resources
 * the order of enums must match with elements in dictionary
 *
 */
enum CD : uint8_t {
    ESPEM_H = (0U),
    ESPEM_DB,
    ESPEMSet,
    MScale,
    Voltage,
    PowerF
};


// Infoclock - English Strings (order does not matther)
// ИнфоЧасики - Русские тексты (порядок значения не имеет)
static const char T_EN_ESPEM[] PROGMEM = "ESP Energy Monitor";
static const char T_RU_ESPEM[] PROGMEM = "ESP Энергометр";
static const char T_EN_ESPEM_DB[] PROGMEM = "ESPEM Dashboard";
static const char T_RU_ESPEM_DB[] PROGMEM = "ESPEM Индикаторы";
static const char T_EN_ESPEMSet[] PROGMEM = "ESPEM Setup";
static const char T_RU_ESPEMSet[] PROGMEM = "ESPEM Параметры";
static const char T_EN_MScale[] PROGMEM = "Metrics scale";
static const char T_RU_MScale[] PROGMEM = "Масштаб выборки";
static const char T_EN_Voltage[] PROGMEM = "Voltage meter";
static const char T_RU_Voltage[] PROGMEM = "Напряжение сети";
static const char T_EN_PowerF[] PROGMEM = "Power Factor";
static const char T_RU_PowerF[] PROGMEM = "Коэффициент мощности";


/**
 *  Dictionary with references to all text resources
 *  it is a two-dim array of pointers to flash strings.
 *  Each row is a set of messages of a given language
 *  Each colums is a language index
 *  Messages indexes of each lang must match each other
 *  it is possible to reuse untraslated mesages from other lang's
 */
static const char *const C_DICT[][UI_DICT_SIZE] PROGMEM = {
// Index 0 - English lang
  { T_EN_ESPEM,
    T_EN_ESPEM_DB,
    T_EN_ESPEMSet,
    T_EN_MScale,
    T_EN_Voltage,
    T_EN_PowerF
  }
// Index 1 - Russian lang
  { T_RU_ESPEM,
    T_RU_ESPEM_DB,
    T_RU_ESPEMSet,
    T_RU_MScale,
    T_RU_Voltage,
    T_RU_PowerF
  },
};