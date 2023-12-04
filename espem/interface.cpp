#include "espem.h"
#include <EmbUI.h>
#include "interface.h"
#include "ui_i18n.h"    // localized GUI text-strings

// статический класс с готовыми формами для базовых системных натсроек
#include "basicui.h"

#define MAX_UI_UPDATE_RATE 30

extern Espem *espem;

static const char* chart_css = "graphwide";

// forward declarations
void ui_page_espem(Interface *interf, const JsonObject *data, const char* action);

/**
 * Headlile section
 * this is an overriden weak method that builds our WebUI from the top
 * ==
 * Головная секция
 * переопределенный метод фреймфорка, который начинает строить корень нашего WebUI
 * 
 */
void ui_page_main(Interface *interf, const JsonObject *data, const char* action){

    interf->json_frame_interface();

    // application manifest
    interf->json_section_manifest(C_DICT[lang][CD::ESPEM_H], embui.macid(), ESPEM_JSAPI_VERSION, FW_VERSION_STRING);    // HEADLINE for WebUI
    interf->json_section_end(); // json_section_manifest

    // load uidata objects
    interf->json_section_uidata();
        interf->uidata_xload(C_espem_ui, "js/espem.ui.json", false, ESPEM_UI_VERSION);
    interf->json_section_end();

    block_menu(interf);                           // Строим UI блок с меню выбора других секций
    interf->json_frame_flush();                   // send frame

    if((WiFi.getMode() & WIFI_MODE_STA)){         // если контроллер не подключен к внешней AP, сразу открываем вкладку с настройками WiFi
        ui_page_espem(interf, nullptr, action);   // construct main page
    } else {
        basicui::page_settings_netw(interf, nullptr, NULL);
    }
}

/**
 * This code builds UI section with menu block on the left
 * 
 */
void block_menu(Interface *interf){
    // создаем меню
    interf->json_section_menu();    // открываем секцию "меню"
    interf->option(A_ui_page_espem,   C_DICT[lang][CD::ESPEM_DB]);          // пункт меню "ESPEM Info"
    interf->option(A_ui_page_espem_setup, C_DICT[lang][CD::ESPEMSet]);         // пункт меню "ESPEM Setup"

    /**
     * добавляем в меню пункт - настройки,
     */
    basicui::menuitem_settings(interf);
    interf->json_section_end();
}


/**
 * This code builds UI section with dashboard
 * 
 */
void ui_page_espem(Interface *interf, const JsonObject *data, const char* action){
    interf->json_frame_interface();
    interf->json_section_main(A_ui_page_espem, C_DICT[lang][CD::ESPEM_H]);

    interf->json_section_line();             // "Live controls"
        interf->checkbox(A_EPOLLENA, (bool)espem->get_uirate(), "Live update", true);   // Meter poller status
        // UI update rate range slider
        interf->range(A_UI_UPDRT, embui.paramVariant(V_UI_UPDRT).as<int>(), 0, MAX_UI_UPDATE_RATE, 1, "UI update rate, sec", true);
    interf->json_section_end();             // end of line

    // Plain values display
    interf->json_section_line();             // "Live controls"
        auto *m = espem->pz->getMetricsPZ004();
        // Widgets & left side menu
        // id, type, value, label, param
        interf->display("pwr",  m->power/10 );                       // Power
        interf->display("cur",  m->asFloat(pzmbus::meter_t::cur));   // Current
        interf->display("enrg", m->energy/1000);                     // Energy
    interf->json_section_end();     // end of line


    interf->json_section_line();
        // id, type, value, label, param
        interf->jscall("gaugeV", C_mkchart, C_DICT[lang][CD::Voltage], chart_css);      // Voltage gauge
        interf->jscall("gaugePF", C_mkchart, C_DICT[lang][CD::PowerF], chart_css);      // Power Factor
    interf->json_section_end();     // end of line

    StaticJsonDocument<64> doc;
    JsonObject params = doc.to<JsonObject>();               // parameters for charts

    params["arg1"] = embui.paramVariant(V_SMPLCNT);         // samples counter
    interf->jscall("gsmini", C_mkchart, "Power chart", chart_css, params);          // Power chart

    // slider for the amount of metric samples to be plotted on a chart
    interf->range(A_SMPLCNT, embui.paramVariant(V_SMPLCNT).as<int>(), 0, (int)espem->ds.getMetricsCap(), 10, C_DICT[lang][CD::MScale], true);

    interf->json_frame_flush();     // flush frame
}

// Create Additional buttons on "Settings" page
void user_settings_frame(Interface *interf, const JsonObject *data, const char* action){
    interf->button(button_t::generic, A_ui_page_espem_setup, "ESPEM");
}

/**
 * ESPEM options setup
 * 
 */
void block_page_espemset(Interface *interf, const JsonObject *data, const char* action){
    interf->json_frame_interface();
        interf->json_section_uidata();
        interf->uidata_pick("espem.ui.settings.cfg");
    interf->json_frame_flush();

    interf->json_frame_value();
        interf->value(V_UART, embui.paramVariant(V_UART).as<int>());    // Uart port
        interf->value(V_RX, embui.paramVariant(V_RX).as<int>());
        interf->value(V_TX, embui.paramVariant(V_TX).as<int>());
        interf->value(V_EOFFSET, espem->ds.getEnergyOffset());
        // TimeSeries capacity
        interf->value(V_TS_T1_CNT, embui.paramVariant(V_TS_T1_CNT).as<int>());
        interf->value(V_TS_T1_INT, embui.paramVariant(V_TS_T1_INT).as<int>());
        interf->value(V_TS_T2_CNT, embui.paramVariant(V_TS_T2_CNT).as<int>());
        interf->value(V_TS_T2_INT, embui.paramVariant(V_TS_T2_INT).as<int>());
        interf->value(V_TS_T3_CNT, embui.paramVariant(V_TS_T3_CNT).as<int>());
        interf->value(V_TS_T3_INT, embui.paramVariant(V_TS_T3_INT).as<int>());
        // collector state
        interf->value(A_ECOLLECTORSTATE, (uint8_t)espem->get_collector_state());
    interf->json_frame_flush();


    interf->json_frame_interface();
    interf->json_section_content();

    auto tsc = espem->ds.getTSC();
    for ( unsigned i=1; i!=4; ++i ){
        char buff[64];
        char key[8];
        std::snprintf(buff, 64, "Used: %hu/%hu, %u kib", tsc.getTSsize(i), tsc.getTScap(i), tsc.getTScap(i) * 28 / 1024);   // sizeof(pz004::metric)
        std::snprintf(key,8, "t%umem", i);
        interf->constant(std::string_view(key), std::string_view(buff));       // capacity and memory usage
    }

    interf->json_frame_flush();


/*
    // replacing page with a new one with settings
    interf->json_section_main(A_ui_page_espem_setup, C_DICT[lang][CD::ESPEMSet]);

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
        interf->number(V_EOFFSET, espem->ds.getEnergyOffset(), "Energy counter offset");
        interf->button(button_t::submit, A_SET_PZOPTS, T_DICT[lang][TD::D_Apply]);
    interf->json_section_end();     // end of "energy"



    interf->spacer("Metrics collector options");
    String _msg("Metrics pool capacity: ");
    _msg += espem->ds.getMetricsSize();
    _msg += "/";
    _msg += espem->ds.getMetricsCap();              // current number of metrics samples
    _msg += " samples";

    interf->constant("mcap", _msg);

    // Button "Apply Metrics pool settings"
    interf->button(button_t::submit, A_set_espem_pool, T_DICT[lang][TD::D_Apply]);

    //
	// Define metrics collector state
	//   0: Disabled, memory released
	//   1: Running and storing metrics in RAM
	//   2: Paused, collecting but not storing, memory reserved 
	//
    interf->select(A_ECOLLECTORSTATE, (uint8_t)espem->get_collector_state(), "Metrics collector status", true);
        interf->option(0, "Disabled");
        interf->option(1, "Running");
        interf->option(2, "Paused");
    interf->json_section_end();     // select

    interf->json_frame_flush();     // flush frame
*/
}


// Callback ACTIONS

/**
 *  Apply espem options values
 */
void set_sampler_opts(Interface *interf, const JsonObject *data, const char* action){
    if (!data) return;
    // save sampling storage capacity values
    SETPARAM(V_TS_T1_CNT);
    SETPARAM(V_TS_T1_INT);
    SETPARAM(V_TS_T2_CNT);
    SETPARAM(V_TS_T2_INT);
    SETPARAM(V_TS_T3_CNT);
    SETPARAM(V_TS_T3_INT);

    espem->ds.reset();
    // display main page
    if (interf) ui_page_espem(interf, nullptr, NULL);
}


/**
 * обработка "живых" переключателей
 * 
 */
void set_directctrls(Interface *interf, const JsonObject *data, const char* action){
    if (!data) return;

    std::string_view sv(action);
    sv.remove_prefix(5);  // 'dctl_'

    // ena/disable polling
    if (sv.compare(V_EPOLLENA) == 0){
        espem->set_uirate( (*data)[A_EPOLLENA] ? embui.paramVariant(V_UI_UPDRT) : 0);
        LOG(printf_P, PSTR("ESPEM: UI refresh state: %d\n"), (*data)[A_EPOLLENA].as<int>() );
        return;
    }

    // UI update rate
    if (sv.compare(V_UI_UPDRT) == 0){
        espem->set_uirate((*data)[A_UI_UPDRT]);
        embui.var(V_UI_UPDRT, espem->get_uirate());
        LOG( printf_P, PSTR("ESPEM: Set UI update rate to: %d\n"), espem->get_uirate() );
        return;
    }

    // Metrics collector run/pause
    if (sv.compare(V_ECOLLECTORSTATE) == 0){
        uint8_t new_state = (*data)[A_ECOLLECTORSTATE];
        // reset TS Container if empty and we need to start it
        //if (espem->get_collector_state() == mcstate_t::MC_DISABLE && new_state >0)
        //    espem->ds.tsSet(embui.paramVariant(V_EPOOLSIZE), embui.paramVariant(V_SMPL_PERIOD));

        espem->set_collector_state((mcstate_t)new_state);
        LOG(printf, "UI: Set TS Collector state to: %d\n", (int)espem->get_collector_state() );
        return;
    }

    // Metrics graph - number of samples to draw in a small power chart
    if (sv.compare(V_SMPLCNT) == 0){
        embui.var(V_SMPLCNT, (*data)[A_SMPLCNT]);
        // send update command to AmCharts block
        if (interf){
            interf->json_frame("rawdata");
            interf->value("scntr", (*data)[A_SMPLCNT]);
            interf->json_frame_flush();
        }
    }
}

/**
 *  Apply uart options values
 */
void set_uart_opts(Interface *interf, const JsonObject *data, const char* action){
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
    if (interf) ui_page_espem(interf, nullptr, NULL);
}

/**
 * @brief Set pzem opts
 * 
 * @param interf 
 * @param data 
 */
void set_pzopts(Interface *interf, const JsonObject *data, const char* action){
    if (!data) return;

    SETPARAM(V_EOFFSET);
    espem->ds.setEnergyOffset(embui.paramVariant(V_EOFFSET));

    // display main page
    if (interf) ui_page_espem(interf, nullptr, NULL);
}


/**
 * Define configuration variables and controls handlers
 * variables has literal names and are kept within json-configuration file on flash
 * 
 * Control handlers are bound by literal name with a particular method. This method is invoked
 * by manipulating controls
 * 
 */
void embui_actions_register(){
    LOG(println, "UI: Creating application vars");

    /**
     * регистрируем свои переменные
     */
    embui.var_create(V_UI_UPDRT, DEFAULT_WS_UPD_RATE);       // WebUI update rate
    // Time Series values
    embui.var_create(V_TS_T1_CNT, TS_T1_CNT);
    embui.var_create(V_TS_T1_INT, TS_T1_INTERVAL);
    embui.var_create(V_TS_T2_CNT, TS_T2_CNT);
    embui.var_create(V_TS_T2_INT, TS_T2_INTERVAL);
    embui.var_create(V_TS_T3_CNT, TS_T3_CNT);
    embui.var_create(V_TS_T3_INT, TS_T3_INTERVAL);
    embui.var_create(V_UART, 0x1);                           // default UART port UART_NUM_1
    embui.var_create(V_RX, -1);                              // RX pin (default)
    embui.var_create(V_TX, -1);                              // TX pin (default)
    embui.var_create(V_TX, -1);                              // TX pin (default)
    embui.var_create(V_EOFFSET, 0);                          // Energy counter offset

    /**
     * обработчики действий
     */ 

    // UI page callback handlers
    embui.action.set_mainpage_cb(ui_page_main);              // index page callback
    embui.action.set_settings_cb(user_settings_frame);       // "settings" page options callback
    //embui.action.set_publish_cb(pubCallback);                // Publish callback

    // вывод WebUI секций
    embui.action.add(A_ui_page_espem, ui_page_espem);                // generate "main" info page
    embui.action.add(A_ui_page_espem_setup, block_page_espemset);       // generate "ESPEM settings" page


    // активности
    embui.action.add(A_SET_MCOLLECTOR,  set_sampler_opts);   // set options for TimeSeries collector
    embui.action.add(A_SET_UART,   set_uart_opts);           // set UART gpios
    embui.action.add(A_SET_PZOPTS,   set_pzopts);            // set options for PZEM (egergy offset)

    // direct controls
    embui.action.add(A_DIRECT_CTL,  set_directctrls);        // process onChange update controls
}
