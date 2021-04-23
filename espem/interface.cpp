#include "main.h"

#include <EmbUI.h>
#include "interface.h"
#include "espem.h"

// статический класс с готовыми формами для базовых системных натсроек
#include "basicui.h"

#define MAX_POLL_PERIOD 60

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
    embui.var_create(FPSTR(V_EPOLLRT), ESPEM_POLLRATE);         // Metrics collector pollrate
    embui.var_create(FPSTR(V_EPFFIX), true);                    // PowerFactor value correction
    embui.var_create(FPSTR(V_EPOLLENA), true);         	        // Meter poller active
    embui.var_create(FPSTR(V_EPOOLSIZE), ESPEM_MEMPOOL);        // metrics collector mem pool size, KiB
    embui.var_create(FPSTR(V_SMPLCNT), 0);                       // Metrics graph - number of samples to draw in a small power chart

    //Metrics collector run/pause
    embui.var_create(FPSTR(V_ECOLLECTORSTATE), 1);               // Collector state

    /**
     * обработчики действий
     */ 
    // вывод WebUI секций
    embui.section_handle_add(FPSTR(B_ESPEM), block_page_main);              // generate "main" info page
    embui.section_handle_add(FPSTR(B_ESPEMSET), block_page_espemset);       // generate "ESPEM settings" page


   /**
    * регистрируем статические секции для web-интерфейса с системными настройками,
    */
    BasicUI::add_sections();


    // активности
    embui.section_handle_add(FPSTR(A_SET_ESPEM),  set_espem_opts);             // save matrix settings

    // direct controls
    embui.section_handle_add(FPSTR(A_DIRECT_CTL),  set_directctrls);             // process checkboxes with direct updates

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
        block_page_main(interf, data);                 // Строим основной блок 
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

    interf->checkbox(FPSTR(V_EPOLLENA), espem->meterPolling(), F("Meter Polling"), true);   // Meter poller status
    // Poll Rate range slider
    interf->range(FPSTR(V_EPOLLRT), embui.param(FPSTR(V_EPOLLRT)).toInt(), 1, MAX_POLL_PERIOD, 1, F("Poll Rate, sec"), true);
    interf->json_section_end();     // end of line

    DynamicJsonDocument doc(128);
    JsonObject params = doc.to<JsonObject>();
    params[F("class")] = F("graphwide");    // css selector

    interf->json_section_line();
    // id, type, value, label, param
    interf->custom(F("gaugeV"), F("div"),  FPSTR(C_mkchart), FPSTR(C_DICT[lang][CD::Voltage]), params);   // Voltage gauge
    interf->custom(F("gaugePF"), F("div"), FPSTR(C_mkchart), FPSTR(C_DICT[lang][CD::PowerF]), params);    // Power Factor
    interf->json_section_end();     // end of line

    params[F("arg1")] = embui.param(FPSTR(V_SMPLCNT));
    interf->custom(F("gsmini"), F("div"), FPSTR(C_mkchart), F("Power chart"), params);

    // slider for the amount of metric samples to be plotted on a chart
    interf->range(FPSTR(V_SMPLCNT), (int)embui.param(FPSTR(V_SMPLCNT)).toInt(), 0, (int)espem->getMetricsCap(), 10, FPSTR(C_DICT[lang][CD::MScale]), true);

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
    //interf->json_section_line(FPSTR(A_SET_ESPEM));
    interf->json_section_line("");
    interf->checkbox(FPSTR(V_EPFFIX), espem->pffix(), F("Power-Factor Fix"), true);
    interf->checkbox(FPSTR(V_EPOLLENA), espem->meterPolling(), F("Meter Polling"), true);   // Meter poller status

    // Poll Rate range slider
    interf->range(FPSTR(V_EPOLLRT), embui.param(FPSTR(V_EPOLLRT)).toInt(), 1, MAX_POLL_PERIOD, 1, F("Poll Rate, sec"), true);
    interf->json_section_end();     // end of line


    interf->spacer(F("Metrics collector options"));         // Weather setup Note
    String _msg(F("Metrics pool capacity: "));
    _msg += espem->getMetricsCap();                         // current number of metrics samples
    _msg += F(" samples");

    interf->constant(F("mcap"), _msg);

    interf->json_section_line(FPSTR(A_SET_ESPEM));
    //interf->text(FPSTR(V_EPOOLSIZE), embui.param(FPSTR(V_EPOOLSIZE)), FPSTR(F("Metrics RAM pool size, KiB")), false);          // Memory pool for metrics data, KiB
    interf->number(FPSTR(V_EPOOLSIZE), FPSTR(F("Metrics RAM pool size, KiB")));          // Memory pool for metrics data, KiB
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
    interf->option("0", F("Disabled"));
    interf->option("1", F("Running"));
    interf->option("2", F("Paused"));
    interf->json_section_end();

    interf->json_section_end();     // end of main
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

    espem->poolResize((*data)[FPSTR(V_EPOOLSIZE)].as<unsigned int>());
    // display main page
    block_page_main(interf, data);
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

        if (!_s.compareTo(_k)){
           espem->pffix(kv.value().as<unsigned short>());
           SETPARAM(FPSTR(V_EPFFIX));
           LOG(printf_P, PSTR("UI: Set PF fix to: %d\n"), espem->pffix() );
           continue;
        }

        _s=FPSTR(V_EPOLLENA);
        if (!_s.compareTo(_k)){
           espem->meterPolling(kv.value().as<unsigned short>());
           SETPARAM(FPSTR(V_EPOLLENA));
           LOG(printf_P, PSTR("UI: Set Polling state to: %d\n"), espem->meterPolling() );
           continue;
        }

        _s=FPSTR(V_EPOLLRT);
        if (!_s.compareTo(_k)){
           espem->PollRate(kv.value().as<unsigned int>());
           SETPARAM(FPSTR(V_EPOLLRT));
           LOG(printf_P, PSTR("UI: Set Poll interval to: %d\n"), espem->PollRate() );
           continue;
        }

        _s=FPSTR(V_ECOLLECTORSTATE);
        if (!_s.compareTo(_k)){
            mcstate_t _a = espem->collector((mcstate_t)kv.value().as<unsigned short>());
            embui.var(_k, String((uint8_t)_a));
            LOG(printf_P, PSTR("UI: Changed Collector state to: %d\n"), (int)espem->collector() );
        }

        _s=FPSTR(V_SMPLCNT);
        if (!_s.compareTo(_k)){
            SETPARAM(FPSTR(V_SMPLCNT));

            if (interf){
                interf->json_frame_custom(F("rawdata"));
                interf->value(F("scntr"), kv.value());
            }
        }
    }

    interf->json_frame_flush();
}
