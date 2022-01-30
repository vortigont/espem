#pragma once

// Interface blocks
void block_menu(Interface *interf, JsonObject *data);
void block_page_main(Interface *interf, JsonObject *data);
void block_page_espemset(Interface *interf, JsonObject *data);

//void remote_action(RA action, ...);
//void uploadProgress(size_t len, size_t total);

//  ACTIONS
void action_demopage(Interface *interf, JsonObject *data);
void set_sampler_opts(Interface *interf, JsonObject *data);
void set_directctrls(Interface *interf, JsonObject *data);
void set_uart_opts(Interface *interf, JsonObject *data);
void set_pzopts(Interface *interf, JsonObject *data);

// Callbacks
void pubCallback(Interface *interf);
