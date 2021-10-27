#include "main.h"

#include <EmbUI.h>
#include "interface.h"
#include "ui_i18n.h"    // localized GUI text-strings
#include "espem.h"

// статический класс с готовыми формами для базовых системных натсроек
#include "basicui.h"

#define MAX_UI_UPDATE_RATE 30

extern ESPEM *espem;

/**
 * Define configuration variables and controls handlers
 * variables has literal names and are kept within json-configuration file on flash
 * 
 * Control handlers are bound by literal name with a particular method. This method is invoked
 * by manipulating controls
 * 
 */
void create_parameters(){
    LOG(println, F("UI: Creating application vars"));

    /**
     * регистрируем свои переменные
     */
    embui.var_create(FPSTR(V_UI_UPDRT), DEFAULT_WS_UPD_RATE);       // WebUI update rate
    embui.var_create(FPSTR(V_SMPL_PERIOD), 1);                      // 
    embui.var_create(FPSTR(V_EPOOLSIZE), ESPEM_MEMPOOL);            // metrics collector mem pool size, KiB
    embui.var_create(FPSTR(V_SMPLCNT), 0);                          // Metrics graph - number of samples to draw in a small power chart

    //Metrics collector run/pause
    //embui.var_create(FPSTR(V_ECOLLECTORSTATE), 1);                  // Collector state

    /**
     * обработчики действий
     */ 
    // вывод WebUI секций
    embui.section_handle_add(FPSTR(B_ESPEM), block_page_main);              // generate "main" info page
    embui.section_handle_add(FPSTR(B_ESPEMSET), block_page_espemset);       // generate "ESPEM settings" page


   /**
    * регистрируем статические секции для web-интерфейса с системными настройками
    */
    BasicUI::add_sections();


    // активности
    embui.section_handle_add(FPSTR(A_SET_ESPEM),  set_espem_opts);

    // direct controls
    embui.section_handle_add(FPSTR(A_DIRECT_CTL),  set_directctrls);             // process direct update controls

}


/**
 * Sync all UI params
 */
void sync_parameters(){}

/**
 * Headlile section
 * this is an overriden weak method that builds our WebUI from the top
 * ==
 * Головная секция
 * переопределенный метод фреймфорка, который начинает строить корень нашего WebUI
 * 
 */
void section_main_frame(Interface *interf, JsonObject *data){
    if (!interf) return;

    interf->json_frame_interface(FPSTR(C_DICT[lang][CD::ESPEM_H]));    // HEADLINE for WebUI
    block_menu(interf, data);                           // Строим UI блок с меню выбора других секций
    interf->json_frame_flush();

    if(!embui.sysData.wifi_sta){                        // если контроллер не подключен к внешней AP, сразу открываем вкладку с настройками WiFi
        LOG(println, F("UI: Opening network setup section"));
        BasicUI::block_settings_netw(interf, data);
    } else {
        block_page_main(interf, data);                  // Строим основной блок 
    }

    interf->json_frame_flush();                         // Close interface section

    // Publish firmware version (visible under menu section)
    interf->json_frame_value();
    interf->value(F("fwver"), F(FW_VERSION_STRING), true);
    interf->json_frame_flush();
}

/**
 * This code builds UI section with menu block on the left
 * 
 */
void block_menu(Interface *interf, JsonObject *data){
    if (!interf) return;
    // создаем меню
    interf->json_section_menu();    // открываем секцию "меню"
    interf->option(FPSTR(B_ESPEM),   FPSTR(C_DICT[lang][CD::ESPEM_DB]));           // пункт меню "ESPEM Info"
    interf->option(FPSTR(B_ESPEMSET), FPSTR(C_DICT[lang][CD::ESPEMSet]));         // пункт меню "ESPEM Setup"

    /**
     * добавляем в меню пункт - настройки,
     */
    BasicUI::opt_setup(interf, data);       // пункт меню "настройки"

    interf->json_section_end();
}


/**
 * This code builds UI section with dashboard
 * 
 */
void block_page_main(Interface *interf, JsonObject *data){
    if (!interf) return;
    interf->json_frame_interface();
    interf->json_section_main(FPSTR(B_ESPEM), FPSTR(C_DICT[lang][CD::ESPEM_H]));

    interf->json_section_line();             // "Live controls"

    interf->checkbox(FPSTR(V_EPOLLENA), (bool)espem->get_uirate(), F("Live update"), true);   // Meter poller status
    // UI update rate range slider
    interf->range(FPSTR(V_UI_UPDRT), embui.paramVariant(FPSTR(V_UI_UPDRT)), 0, MAX_UI_UPDATE_RATE, 1, F("UI update rate, sec"), true);
    interf->json_section_end();     // end of line

    // Plain values display
    interf->json_section_line();             // "Live controls"
    auto *m = espem->pz->getMetricsPZ004();
    // id, type, value, label, param
    interf->display(F("pwr"),  m->power/10 );      // Power
    interf->display(F("cur"),  m->asFloat(pzmbus::meter_t::cur));   // Current
    interf->display(F("enrg"), m->energy/1000);    // Energy
    interf->json_section_end();     // end of line


    StaticJsonDocument<128> doc;
    JsonObject params = doc.to<JsonObject>();
    params[F("class")] = F("graphwide");    // css selector

    interf->json_section_line();
    // id, type, value, label, param
    interf->custom(F("gaugeV"), FPSTR(C_js),  FPSTR(C_mkchart), FPSTR(C_DICT[lang][CD::Voltage]), params);   // Voltage gauge
    interf->custom(F("gaugePF"), FPSTR(C_js), FPSTR(C_mkchart), FPSTR(C_DICT[lang][CD::PowerF]), params);    // Power Factor
    interf->json_section_end();     // end of line

    params[F("arg1")] = embui.paramVariant(FPSTR(V_SMPLCNT));
    interf->custom(F("gsmini"), FPSTR(C_js), FPSTR(C_mkchart), F("Power chart"), params);

    // slider for the amount of metric samples to be plotted on a chart
    interf->range(FPSTR(V_SMPLCNT), embui.paramVariant(FPSTR(V_SMPLCNT)).as<int>(), 0, (int)espem->getMetricsCap(), 10, FPSTR(C_DICT[lang][CD::MScale]), true);

    interf->json_frame_flush();     // flush frame
}

/**
 * ESPEM options setup
 * 
 */
void block_page_espemset(Interface *interf, JsonObject *data){
    if (!interf) return;
    interf->json_frame_interface();

    // replacing page with a new one with settings
    //interf->json_section_main(FPSTR(A_SET_ESPEM), FPSTR(C_DICT[lang][CD::ESPEMSet]));
    interf->json_section_main("", FPSTR(C_DICT[lang][CD::ESPEMSet]));

    // Poller Line block
    /*
    interf->json_section_line("");
    interf->checkbox(FPSTR(V_EPOLLENA), espem->meterPolling(), F("Meter Polling"), true);   // Meter poller status

    // UI update Rate range slider
    interf->range(FPSTR(V_UI_UPDRT), embui.paramVariant(FPSTR(V_UI_UPDRT)).as<int>(), 0, MAX_UI_UPDATE_RATE, 1, F("UI refresh rate, sec"), true);
    interf->json_section_end();     // end of line
    */

    interf->spacer(F("Metrics collector options"));
    String _msg(F("Metrics pool capacity: "));
    _msg += espem->getMetricsSize();
    _msg += "/";
    _msg += espem->getMetricsCap();                         // current number of metrics samples
    _msg += F(" samples");

    interf->constant(F("mcap"), _msg);

    interf->json_section_line(FPSTR(A_SET_ESPEM));
    interf->number(FPSTR(V_EPOOLSIZE), F("RAM pool size, samples"));          // Memory pool for metrics data, samples
    interf->number(FPSTR(V_SMPL_PERIOD), "Sampling period");                  // sampling period, sec
    // Button "Apply Metrics pool settings"
    interf->button_submit(FPSTR(A_SET_ESPEM), FPSTR(T_DICT[lang][TD::D_Apply]), F("blue"));
    interf->json_section_end();     // end of line

    /*
	 * Define metrics collector state
	 *   0: Disabled, memory released
	 *   1: Running and storing metrics in RAM
	 *   2: Paused, collecting but not storing, memory reserved 
	 */
    interf->select(FPSTR(V_ECOLLECTORSTATE), embui.param(FPSTR(V_ECOLLECTORSTATE)), F("Metrics collector status"), true, false);
    interf->option(0, F("Disabled"));
    interf->option(1, F("Running"));
    interf->option(2, F("Paused"));
    interf->json_section_end();     // select

    interf->json_frame_flush();     // flush frame
}


/**
 * обработчик статуса (периодического опроса контроллера веб-приложением)
 */
void pubCallback(Interface *interf){
    BasicUI::embuistatus(interf);
}


// Callback ACTIONS

/**
 *  Apply espem options values
 */
void set_espem_opts(Interface *interf, JsonObject *data){
    if (!data) return;

    SETPARAM(FPSTR(V_EPOOLSIZE));
    SETPARAM(FPSTR(V_SMPL_PERIOD));

    espem->tsSet((*data)[FPSTR(V_EPOOLSIZE)].as<unsigned int>(), (*data)[FPSTR(V_SMPL_PERIOD)].as<unsigned int>());
    // display main page
    if (interf) block_page_main(interf, nullptr);
}


/**
 * обработка "живых" переключателей
 * 
 */
void set_directctrls(Interface *interf, JsonObject *data){
    if (!data) return;

    for (JsonPair kv : (*data)) {

        //LOG(printf_P, PSTR("Iterating Key:%s Value:%s\n"), kv.key().c_str(), kv.value().as<char*>() );

        String _s(FPSTR(V_EPFFIX));
        String _k(kv.key().c_str());

        _s=FPSTR(V_EPOLLENA);
        if (!_s.compareTo(_k)){
           if(kv.value()){
               espem->set_uirate(embui.paramVariant(FPSTR(V_UI_UPDRT)));
           } else {
               espem->set_uirate(0);
           }
           LOG(printf_P, PSTR("ESPEM: UI refresh state: %d\n"), kv.value().as<int>() );
           continue;
        }

        _s=FPSTR(V_UI_UPDRT);
        if (!_s.compareTo(_k)){
           espem->set_uirate(kv.value().as<unsigned short>());
           SETPARAM(FPSTR(V_UI_UPDRT));
           LOG( printf_P, PSTR("ESPEM: Set UI update rate to: %d\n"), espem->get_uirate() );
           continue;
        }


        _s=FPSTR(V_ECOLLECTORSTATE);
        if (!_s.compareTo(_k)){
            uint8_t new_state = kv.value().as<unsigned short>();
            // reset TS Container if empty and we need to start it
            if (espem->get_collector_state() == mcstate_t::MC_DISABLE && new_state >0) espem->tsSet(embui.paramVariant(FPSTR(V_EPOOLSIZE)), embui.paramVariant(FPSTR(V_SMPL_PERIOD)));

            embui.var(_k, (uint8_t)espem->set_collector_state((mcstate_t)new_state));
            LOG(printf_P, PSTR("UI: Set TS Collector state to: %d\n"), (int)espem->get_collector_state() );
            continue;
        }


        // Set amount of samples displayed on chart (TODO: replace with js internal var)
        _s=FPSTR(V_SMPLCNT);
        if (!_s.compareTo(_k)){
            SETPARAM(FPSTR(V_SMPLCNT));

            if (interf){
                interf->json_frame_custom(F("rawdata"));
                interf->value(F("scntr"), kv.value());
                interf->json_frame_flush();
            }
        }
    }

}
