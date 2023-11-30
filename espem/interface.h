#pragma once

// register config params and action callbacks
void embui_actions_register();

// Interface blocks
void block_menu(Interface *interf);
void block_page_main(Interface *interf, const JsonObject *data, const char* action);
void block_page_espemset(Interface *interf, const JsonObject *data, const char* action);

//  ACTIONS
void action_demopage(Interface *interf, const JsonObject *data, const char* action);
void set_sampler_opts(Interface *interf, const JsonObject *data, const char* action);
void set_directctrls(Interface *interf, const JsonObject *data, const char* action);
void set_uart_opts(Interface *interf, const JsonObject *data, const char* action);
void set_pzopts(Interface *interf, const JsonObject *data, const char* action);

// Callbacks
void pubCallback(Interface *interf);
