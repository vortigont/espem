// localization resources goes here

#pragma once

/**
 *  Dictionary size
 *  must be more or equal to the number of messages in TD Enum
 */
#define UI_DICT_SIZE 20

/**
 * Clock Text-Dictionary Enums for language resources
 * the order of enums must match with elements in dictionary
 *
 */
enum CD : uint8_t {
    ESPEMInf = (0U),
    ESPEMSet
};


// Infoclock - English Strings (order does not matther)
// ИнфоЧасики - Русские тексты (порядок значения не имеет)
static const char T_EN_ESPEM[] PROGMEM = "Dashboard";
static const char T_RU_ESPEM[] PROGMEM = "Индикаторы";
static const char T_EN_ESPEMSet[] PROGMEM = "ESPEM Setup";
static const char T_RU_ESPEMSet[] PROGMEM = "ESPEM Параметры";

/**
 *  Dictionary with references to all text resources
 *  it is a two-dim array of pointers to flash strings.
 *  Each row is a set of messages of a given language
 *  Each colums is a language index
 *  Messages indexes of each lang must match each other
 *  it is possible to reuse untraslated mesages from other lang's
 */
static const char *const C_DICT[][UI_DICT_SIZE] PROGMEM = {
// Index 0 - Russian lang
  { T_RU_ESPEM,
    T_RU_ESPEMSet,
  },
// Index 1 - English lang
  { T_EN_ESPEM,
    T_EN_ESPEMSet,
  }
};