#include "espem.h"
#include <EmbUI.h>
#include "interface.h"
#include "ui_i18n.h"    // localized GUI text-strings

// статический класс с готовыми формами для базовых системных натсроек
#include "basicui.h"

#define MAX_UI_UPDATE_RATE 30

extern ESPEM *espem;

static const char* chart_css = "graphwide";

/**
 * Define configuration variables and controls handlers
 * variables has literal names and are kept within json-configuration file on flash
 * 
 * Control handlers are bound by literal name with a particular method. This method is invoked
 * by manipulating controls
 * 
 */
void create_parameters(){
    LOG(println, "UI: Creating application vars");

    /**
     * регистрируем свои переменные
     */
    embui.var_create(V_UI_UPDRT, DEFAULT_WS_UPD_RATE);       // WebUI update rate
    embui.var_create(V_SMPL_PERIOD, 1);                      // 
    embui.var_create(V_EPOOLSIZE, ESPEM_MEMPOOL);            // metrics collector mem pool size, KiB
    embui.var_create(V_UART, 0x1);                           // default UART port UART_NUM_1
    embui.var_create(V_RX, -1);                              // RX pin (default)
    embui.var_create(V_TX, -1);                              // TX pin (default)
    embui.var_create(V_TX, -1);                              // TX pin (default)
    embui.var_create(V_EOFFSET, 0.0);                        // Energy counter offset


    //Metrics collector run/pause
    //embui.var_create(V_ECOLLECTORSTATE, 1);                  // Collector state

    /**
     * обработчики действий
     */ 
    // вывод WebUI секций
    embui.section_handle_add(B_ESPEM, block_page_main);              // generate "main" info page
    embui.section_handle_add(B_ESPEMSET, block_page_espemset);       // generate "ESPEM settings" page


   /**
    * регистрируем статические секции для web-интерфейса с системными настройками
    */
    basicui::add_sections();


    // активности
    embui.section_handle_add(A_SET_ESPEM,  set_sampler_opts);
    embui.section_handle_add(A_SET_UART,   set_uart_opts);
    embui.section_handle_add(A_SET_PZOPTS,   set_pzopts);

    // direct controls
    embui.section_handle_add(A_DIRECT_CTL,  set_directctrls);             // process direct update controls


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

    interf->json_frame_interface();

    // application manifest
    interf->json_section_manifest(C_DICT[lang][CD::ESPEM_H], 0, FW_VERSION_STRING);    // HEADLINE for WebUI
    interf->json_section_end(); // json_section_manifest

    block_menu(interf, data);                           // Строим UI блок с меню выбора других секций
    interf->json_frame_flush();                         // send frame

    if(!(WiFi.getMode() & WIFI_MODE_STA)){                        // если контроллер не подключен к внешней AP, сразу открываем вкладку с настройками WiFi
        LOG(println, "UI: Opening network setup section");
        basicui::block_settings_netw(interf, data);
    } else {
        block_page_main(interf, data);                  // Строим основной блок 
    }

}

/**
 * This code builds UI section with menu block on the left
 * 
 */
void block_menu(Interface *interf, JsonObject *data){
    if (!interf) return;
    // создаем меню
    interf->json_section_menu();    // открываем секцию "меню"
    interf->option(B_ESPEM,   C_DICT[lang][CD::ESPEM_DB]);           // пункт меню "ESPEM Info"
    interf->option(B_ESPEMSET, C_DICT[lang][CD::ESPEMSet]);         // пункт меню "ESPEM Setup"

    /**
     * добавляем в меню пункт - настройки,
     */
    basicui::opt_setup(interf, data);       // пункт меню "настройки"

    interf->json_section_end();
}


/**
 * This code builds UI section with dashboard
 * 
 */
void block_page_main(Interface *interf, JsonObject *data){
    if (!interf) return;
    interf->json_frame_interface();
    interf->json_section_main(B_ESPEM, C_DICT[lang][CD::ESPEM_H]);

    interf->json_section_line();             // "Live controls"

    interf->checkbox(V_EPOLLENA, (bool)espem->get_uirate(), "Live update", true);   // Meter poller status
    // UI update rate range slider
    interf->range(V_UI_UPDRT, embui.paramVariant(V_UI_UPDRT).as<int>(), 0, MAX_UI_UPDATE_RATE, 1, "UI update rate, sec", true);
    interf->json_section_end();     // end of line

    // Plain values display
    interf->json_section_line();             // "Live controls"
    auto *m = espem->pz->getMetricsPZ004();
    // Widgets & left side menu
    // id, type, value, label, param
    interf->display("pwr",  m->power/10 );                       // Power
    interf->display("cur",  m->asFloat(pzmbus::meter_t::cur));   // Current
    interf->display("enrg", m->energy/1000);                     // Energy
    interf->json_section_end();     // end of line


    StaticJsonDocument<64> doc;
    JsonObject params = doc.to<JsonObject>();   // parameters for charts

    interf->json_section_line();
    // id, type, value, label, param
    interf->jscall("gaugeV", C_mkchart, C_DICT[lang][CD::Voltage], chart_css);      // Voltage gauge
    interf->jscall("gaugePF", C_mkchart, C_DICT[lang][CD::PowerF], chart_css);      // Power Factor
    interf->json_section_end();     // end of line

    params["arg1"] = embui.paramVariant(V_SMPLCNT);      // samples counter
    interf->jscall("gsmini", C_mkchart, "Power chart", chart_css, params);          // Power chart

    // slider for the amount of metric samples to be plotted on a chart
    interf->range(V_SMPLCNT, embui.paramVariant(V_SMPLCNT).as<int>(), 0, (int)espem->getMetricsCap(), 10, C_DICT[lang][CD::MScale], true);

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
    //interf->json_section_main(A_SET_ESPEM, C_DICT[lang][CD::ESPEMSet]);
    interf->json_section_main("", C_DICT[lang][CD::ESPEMSet]);

    // Poller Line block
    /*
    interf->json_section_line("");
    interf->checkbox(V_EPOLLENA, espem->meterPolling(), "Meter Polling", true);   // Meter poller status

    // UI update Rate range slider
    interf->range(V_UI_UPDRT, embui.paramVariant(V_UI_UPDRT).as<int>(), 0, MAX_UI_UPDATE_RATE, 1, "UI refresh rate, sec", true);
    interf->json_section_end();     // end of line
    */

    interf->json_section_begin(A_SET_UART);

        interf->json_section_line();
            interf->number_constrained(V_UART, embui.paramVariant(V_UART).as<int>(), "Uart port", 1, 0, SOC_UART_NUM);
            interf->number_constrained(V_RX, embui.paramVariant(V_RX).as<int>(), "RX pin (-1 default)", 1, -1, NUM_OUPUT_PINS);
            interf->number_constrained(V_TX, embui.paramVariant(V_TX).as<int>(), "TX pin (-1 default)", 1, -1, NUM_OUPUT_PINS);
        interf->json_section_end();     // end of line

        interf->button(button_t::submit, A_SET_UART, T_DICT[lang][TD::D_Apply]);
    interf->json_section_end();     // end of "uart"

    // counter opts
    interf->spacer("Energy counter options");
    interf->json_section_begin(A_SET_PZOPTS);
        interf->number(V_EOFFSET, espem->getEnergyOffset(), "Energy counter offset");
        interf->button(button_t::submit, A_SET_PZOPTS, T_DICT[lang][TD::D_Apply]);
    interf->json_section_end();     // end of "energy"



    interf->spacer("Metrics collector options");
    String _msg("Metrics pool capacity: ");
    _msg += espem->getMetricsSize();
    _msg += "/";
    _msg += espem->getMetricsCap();                         // current number of metrics samples
    _msg += " samples";

    interf->constant("mcap", _msg);

    interf->json_section_line(A_SET_ESPEM);
        interf->number(V_EPOOLSIZE, embui.paramVariant(V_EPOOLSIZE).as<int>(), "RAM pool size, samples");          // Memory pool for metrics data, samples
        interf->number(V_SMPL_PERIOD, embui.paramVariant(V_SMPL_PERIOD).as<int>(), "Sampling period");                  // sampling period, sec
    interf->json_section_end();     // end of line
    // Button "Apply Metrics pool settings"
    interf->button(button_t::submit, A_SET_ESPEM, T_DICT[lang][TD::D_Apply]);

    /*
	 * Define metrics collector state
	 *   0: Disabled, memory released
	 *   1: Running and storing metrics in RAM
	 *   2: Paused, collecting but not storing, memory reserved 
	 */
    interf->select(V_ECOLLECTORSTATE, (uint8_t)espem->get_collector_state(), "Metrics collector status", true);
        interf->option(0, "Disabled");
        interf->option(1, "Running");
        interf->option(2, "Paused");
    interf->json_section_end();     // select

    interf->json_frame_flush();     // flush frame
}


/**
 * обработчик статуса (периодического опроса контроллера веб-приложением)
 */
void pubCallback(Interface *interf){
    basicui::embuistatus(interf);
}


// Callback ACTIONS

/**
 *  Apply espem options values
 */
void set_sampler_opts(Interface *interf, JsonObject *data){
    if (!data) return;

    SETPARAM(V_EPOOLSIZE);
    SETPARAM(V_SMPL_PERIOD);

    espem->tsSet((*data)[V_EPOOLSIZE].as<unsigned int>(), (*data)[V_SMPL_PERIOD].as<unsigned int>());
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

        String _s(V_EPFFIX);
        String _k(kv.key().c_str());

        _s=V_EPOLLENA;
        if (!_s.compareTo(_k)){
           if(kv.value()){
               espem->set_uirate(embui.paramVariant(V_UI_UPDRT));
           } else {
               espem->set_uirate(0);
           }
           LOG(printf_P, PSTR("ESPEM: UI refresh state: %d\n"), kv.value().as<int>() );
           continue;
        }

        _s=V_UI_UPDRT;
        if (!_s.compareTo(_k)){
           espem->set_uirate(kv.value().as<unsigned short>());
           SETPARAM(V_UI_UPDRT);
           LOG( printf_P, PSTR("ESPEM: Set UI update rate to: %d\n"), espem->get_uirate() );
           continue;
        }


        _s=V_ECOLLECTORSTATE;
        if (!_s.compareTo(_k)){
            uint8_t new_state = kv.value().as<unsigned short>();
            // reset TS Container if empty and we need to start it
            if (espem->get_collector_state() == mcstate_t::MC_DISABLE && new_state >0) espem->tsSet(embui.paramVariant(V_EPOOLSIZE), embui.paramVariant(V_SMPL_PERIOD));

            espem->set_collector_state((mcstate_t)new_state);
            //embui.var(_k, (uint8_t)espem->set_collector_state((mcstate_t)new_state), true);   // no need to set this var, it's a run-time state
            LOG(printf_P, PSTR("UI: Set TS Collector state to: %d\n"), (int)espem->get_collector_state() );
            continue;
        }


        // Set amount of samples displayed on chart (TODO: replace with js internal var)
        _s=V_SMPLCNT;
        if (!_s.compareTo(_k)){
            SETPARAM(V_SMPLCNT);

            if (interf){
                interf->json_frame("rawdata");
                interf->value("scntr", kv.value());
                interf->json_frame_flush();
            }
        }
    }

}

/**
 *  Apply uart options values
 */
void set_uart_opts(Interface *interf, JsonObject *data){
    if (!data) return;

    uint8_t p = (*data)[V_UART].as<unsigned short>();
    if ( p <= SOC_UART_NUM ){
        SETPARAM(V_UART);
    } else return;

    int r = (*data)[V_RX].as<int>();
    if (r <= NUM_OUPUT_PINS && r >0) {
        SETPARAM(V_RX);
    } else return;

    int t = (*data)[V_TX].as<int>();
    if (t <= NUM_OUPUT_PINS && r >0){
        SETPARAM(V_TX);
    } else return;

    espem->begin(embui.paramVariant(V_UART), embui.paramVariant(V_RX), embui.paramVariant(V_TX));
    // display main page
    if (interf) block_page_main(interf, nullptr);
}

/**
 * @brief Set pzem opts
 * 
 * @param interf 
 * @param data 
 */
void set_pzopts(Interface *interf, JsonObject *data){
    if (!data) return;

    SETPARAM(V_EOFFSET);
    espem->setEnergyOffset(embui.paramVariant(V_EOFFSET));

    // display main page
    if (interf) block_page_main(interf, nullptr);
}
