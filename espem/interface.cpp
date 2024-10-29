#include "espem.h"
#include <EmbUI.h>
#include "interface.h"
#include "ui_i18n.h"    // localized GUI text-strings

// EmbUI's basic interface
#include "basicui.h"

#define MAX_UI_UPDATE_RATE 30
#define DS_ENTRY_SIZE      28

// our espem object
extern Espem *espem;

static constexpr const char* WebUI = "WebUI";
static constexpr const char* chart_css = "graphwide";

// variable that holds TS id which is currently displayed at Web UI
unsigned power_chart_id{1};

// forward declarations
void ui_page_espem(Interface *interf, JsonObjectConst data, const char* action);

/**
 * Main ESPEM WebUI page
 * ==
 * Головная секция
 * 
 */
void ui_page_main(Interface *interf, JsonObjectConst data, const char* action){

    interf->json_frame_interface();

    // application manifest
    interf->json_section_manifest(C_EspEM, embui.macid(), ESPEM_JSAPI_VERSION, FW_VERSION_STRING);    // HEADLINE for WebUI
    interf->json_section_end();

    block_menu(interf);                           // Build UI Menu block
    interf->json_frame_flush();                   // send frame

    if((WiFi.getMode() & WIFI_MODE_STA)){         // если контроллер не подключен к внешней AP, сразу открываем вкладку с настройками WiFi
        ui_page_espem(interf, {}, action);          // construct main page
    } else {
        // Open WiFi setup page
        basicui::page_settings_netw(interf, {}, NULL);
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
    interf->option(A_ui_page_espem_setup, C_DICT[lang][CD::ESPEMSet]);      // пункт меню "ESPEM Setup"
    interf->option(A_ui_page_dataexport, "DataExport");               // пункт меню "ESPEM Data Export"

    /**
     * добавляем в меню пункт - настройки,
     */
    basicui::menuitem_settings(interf);
    interf->json_section_end();
}


/**
 * @brief create and send jscall frame thet will trigger building power-chart on WebUI
 * 
 * @param interf 
 */
void ui_frame_mkchart(Interface *interf){
    interf->json_frame_jscall(C_mkchart);
    JsonObject params(interf->json_object_create());
    params[P_id] = C_smpchart;
    params[C_tier] = power_chart_id;
    auto ts = espem->ds.getTS(power_chart_id);
    // check if requested TimeSeries exist
    if (ts)
        params["interval"] = ts->getInterval();
    params[C_scnt] = embui.getConfig()[V_SMPLCNT].as<int>() | espem->ds.getTScap(power_chart_id);   // espem->ds.getTScap(power_chart_id);    // samples counter
    interf->json_frame_flush();     // flush frame
}

/**
 * @brief create/replace UI section with power chart controls
 * 
 * @param interf 
 */
void ui_block_chart_ctrls(Interface *interf){
    interf->json_section_line(C_lchart);            // chart Live controls
        interf->select(A_TS_TIER, power_chart_id, "TimeSeries Interval", true);
            for (unsigned i = 0; i != espem->ds.getTScnt(); ++i){
                String lbl(espem->ds.getTS(i+1)->getInterval());
                lbl += " sec.";
                interf->option(i+1, lbl);           // ids are starting from 1
            }
        interf->json_section_end();                 // end select drop-down

        // slider for the amount of metric samples to be plotted on a chart
        //espem->ds.getTS(i+1)
        interf->range(
            A_SMPLCNT,                                      // id
            embui.getConfig()[V_SMPLCNT].as<int>() | 50,    // value
            0,                                              // min
            espem->ds.getTScap(power_chart_id),             // max
            espem->ds.getTScap(power_chart_id)/100,         // step
            C_DICT[lang][CD::MScale], true                  // label
        );
    interf->json_section_end();     // end of line
}

/**
 * @brief send values for power chart controls
 * 
 * @param interf 
 */
void ui_block_chart_ctrls_values(Interface *interf){
    interf->json_frame_value();
        interf->value(A_TS_TIER, power_chart_id);
        interf->value(A_SMPLCNT, embui.getConfig()[V_SMPLCNT].as<int>() | 50);
    interf->json_frame_flush();
}

/**
 * This code builds UI page with dashboard
 * @note called from a ui_page_main()
 * 
 */
void ui_page_espem(Interface *interf, JsonObjectConst data, const char* action){
    interf->json_frame_interface();

    interf->json_section_uidata();
        interf->uidata_pick("espem.ui.pages.main");
    interf->json_section_end();

    //interf->spacer("Power chart spacer");

    // draw block with chart controls
    ui_block_chart_ctrls(interf);

    // div placeholder for TimeSeries Power chart
    //interf->jscall(C_gsmini, P_EMPTY, P_EMPTY, chart_css);
    //interf->div(C_gsmini, P_html, P_EMPTY, P_EMPTY, chart_css);
    interf->json_section_begin(C_minichart);
        interf->div(C_smpchart, "chart", P_EMPTY, "Power Sampling", chart_css);
        interf->spacer("Power Chart");

    // send values for controls
    auto m = espem->pz->getMetricsPZ004();
    interf->json_frame_value();
        interf->value(A_EPOLLENA, (bool)espem->get_uirate());
        interf->value(A_UI_UPDRT, embui.getConfig()[V_UI_UPDRT].as<int>() | DEFAULT_WS_UPD_RATE);

    // call js function to build power chart
    ui_frame_mkchart(interf);

}


/**
 * @brief A callback function
 * Creates Additional buttons on EmbUI's "Settings" page
 */
void block_user_settings(Interface *interf, JsonObjectConst data, const char* action){
    interf->button(button_t::generic, A_ui_page_espem_setup, "ESPEM");
}

/**
 * ESPEM configuration options page
 * 
 */
void block_page_espemset(Interface *interf, JsonObjectConst data, const char* action){
    interf->json_frame_interface();
        interf->json_section_uidata();
        interf->uidata_pick("espem.ui.pages.settings");
    interf->json_frame_flush();

    interf->json_frame_value();
        interf->value(V_UART, embui.getConfig()[V_UART].as<int>() | UART_NUM_1);    // Uart port
        interf->value(V_RX, embui.getConfig()[V_RX].as<int>());
        interf->value(V_TX, embui.getConfig()[V_TX].as<int>());
        interf->value(V_EOFFSET, espem->ds.getEnergyOffset());
        // TimeSeries capacity
        interf->value(V_TS_T1_CNT, embui.getConfig()[V_TS_T1_CNT].as<int>());
        interf->value(V_TS_T1_INT, embui.getConfig()[V_TS_T1_INT].as<int>());
        interf->value(V_TS_T2_CNT, embui.getConfig()[V_TS_T2_CNT].as<int>());
        interf->value(V_TS_T2_INT, embui.getConfig()[V_TS_T2_INT].as<int>());
        interf->value(V_TS_T3_CNT, embui.getConfig()[V_TS_T3_CNT].as<int>());
        interf->value(V_TS_T3_INT, embui.getConfig()[V_TS_T3_INT].as<int>());
        // collector state
        interf->value(A_ECOLLECTORSTATE, (uint8_t)espem->get_collector_state());
    interf->json_frame_flush();


    interf->json_frame_interface();
    interf->json_section_content();

    for ( unsigned i=1; i!=4; ++i ){
        char buff[64];
        char key[8];
        std::snprintf(buff, 64, "Used: %hu/%hu, %u KiB", espem->ds.getTSsize(i), espem->ds.getTScap(i), espem->ds.getTScap(i) * DS_ENTRY_SIZE / 1024);   // sizeof(pz004::metric)
        std::snprintf(key,8, "t%umem", i);
        interf->constant(std::string_view(key), std::string_view(buff));       // capacity and memory usage
    }

    interf->json_frame_flush();
}

/**
 * ESPEM data export page
 * 
 */
void ui_page_dataexport(Interface *interf, JsonObjectConst data, const char* action){
    interf->json_frame_interface();
        interf->json_section_uidata();
        interf->uidata_pick("espem.ui.pages.export");
    interf->json_frame_flush();
}

// Callback ACTIONS

/**
 *  Apply espem options values
 */
void set_sampler_opts(Interface *interf, JsonObjectConst data, const char* action){
    if (!data.size()) return;
    // save sampling storage capacity values
    embui.getConfig()[V_TS_T1_CNT] = data[V_TS_T1_CNT];
    embui.getConfig()[V_TS_T1_INT] = data[V_TS_T1_INT];
    embui.getConfig()[V_TS_T2_CNT] = data[V_TS_T2_CNT];
    embui.getConfig()[V_TS_T2_INT] = data[V_TS_T2_INT];
    embui.getConfig()[V_TS_T3_CNT] = data[V_TS_T3_CNT];
    embui.getConfig()[V_TS_T3_INT] = data[V_TS_T3_INT];

    espem->ds.reset();
    // display main page
    if (interf) ui_page_espem(interf, {}, NULL);
}


/**
 * обработка "живых" переключателей
 * 
 */
void set_directctrls(Interface *interf, JsonObjectConst data, const char* action){
    if (!data) return;

    std::string_view sv(action);
    sv.remove_prefix(5);  // 'dctl_'

    // ena/disable polling
    if (sv.compare(V_EPOLLENA) == 0){
        espem->set_uirate( data[action] ? embui.getConfig()[V_UI_UPDRT] : 0);
        LOGI(WebUI, printf, "UI refresh state: %d\n", data[A_EPOLLENA].as<int>() );
        return;
    }

    // UI update rate
    if (sv.compare(V_UI_UPDRT) == 0){
        espem->set_uirate(data[action]);
        embui.getConfig()[V_UI_UPDRT] = espem->get_uirate();
        LOGI(WebUI,  printf, "Set UI update rate to: %d\n", espem->get_uirate() );
        return;
    }

    // Metrics collector run/pause
    if (sv.compare(V_ECOLLECTORSTATE) == 0){
        uint8_t new_state = data[action];
        // reset TS Container if empty and we need to start it
        //if (espem->get_collector_state() == mcstate_t::MC_DISABLE && new_state >0)
        //    espem->ds.tsSet(embui.getConfig()[V_EPOOLSIZE], embui.getConfig()[V_SMPL_PERIOD]);

        espem->set_collector_state((mcstate_t)new_state);
        LOGD(WebUI, printf, "UI: Set TS Collector state to: %d\n", (int)espem->get_collector_state() );
        return;
    }

    // Metrics graph - number of samples to draw in a small power chart
    if (sv.compare(C_scnt) == 0){
        embui.getConfig()[V_SMPLCNT] = data[action];
        // send update command to AmCharts block
        if (interf){
            interf->json_frame(C_espem);
            interf->value(C_scnt, data[action]);
            interf->json_frame_flush();
        }
        return;
    }

    // select another tier on minichart
    if (sv.compare(C_tier) == 0){
        // save new TS id
        power_chart_id = data[action];
        LOGI(WebUI,  printf, "Switch TS tier to:%d\n", power_chart_id );
        if (!interf) return;
        // publish control values
        ui_block_chart_ctrls_values(interf);
        // call js function to re-build power chart
        ui_frame_mkchart(interf);
        return;
    }
}

/**
 *  Apply uart options values
 */
void set_uart_opts(Interface *interf, JsonObjectConst data, const char* action){
    if (!data) return;

    uint8_t p = data[V_UART].as<unsigned short>();
    if ( p <= SOC_UART_NUM ){
        embui.getConfig()[V_UART] = data[V_UART];
    } else return;

    int r = data[V_RX].as<int>();
    if (r <= NUM_OUPUT_PINS && r >0) {
        embui.getConfig()[V_RX] = data[V_RX];
    } else return;

    int t = data[V_TX].as<int>();
    if (t <= NUM_OUPUT_PINS && r >0){
        embui.getConfig()[V_TX] = data[V_TX];
    } else return;

    espem->begin(embui.getConfig()[V_UART], embui.getConfig()[V_RX], embui.getConfig()[V_TX]);
    // display main page
    if (interf) ui_page_espem(interf, {}, NULL);
}

/**
 * @brief Set pzem opts
 * 
 * @param interf 
 * @param data 
 */
void set_pzopts(Interface *interf, JsonObjectConst data, const char* action){
    if (!data) return;

    embui.getConfig()[V_EOFFSET] = data[V_EOFFSET];
    espem->ds.setEnergyOffset(embui.getConfig()[V_EOFFSET]);

    // display main page
    if (interf) ui_page_espem(interf, {}, NULL);
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
    LOGD(WebUI, println, "UI: Creating application vars");

    /**
     * обработчики действий
     */ 

    // UI page callback handlers
    embui.action.set_mainpage_cb(ui_page_main);              // index page callback
    embui.action.set_settings_cb(block_user_settings);       // "settings" page options callback
    //embui.action.set_publish_cb(pubCallback);                // Publish callback

    // вывод WebUI секций
    embui.action.add(A_ui_page_espem, ui_page_espem);               // generate "main" info page
    embui.action.add(A_ui_page_espem_setup, block_page_espemset);   // generate "ESPEM settings" page
    embui.action.add(A_ui_page_dataexport, ui_page_dataexport);     // generate "Data export" page

    // активности
    embui.action.add(A_SET_MCOLLECTOR,  set_sampler_opts);   // set options for TimeSeries collector
    embui.action.add(A_SET_UART,   set_uart_opts);           // set UART gpios
    embui.action.add(A_SET_PZOPTS,   set_pzopts);            // set options for PZEM (egergy offset)

    // direct controls
    embui.action.add(A_DIRECT_CTL,  set_directctrls);        // process onChange update controls
}
