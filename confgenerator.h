// This file is autogenerated by VESC Tool

#ifndef CONFGENERATOR_H_
#define CONFGENERATOR_H_

#include "datatypes.h"
#include <stdint.h>
#include <stdbool.h>

// Constants
#define MCCONF_SIGNATURE		2211848314
#define APPCONF_SIGNATURE		3264926020

// Functions
int32_t confgenerator_serialize_mcconf(uint8_t *buffer, const mc_configuration *conf);
int32_t confgenerator_serialize_appconf(uint8_t *buffer, const app_configuration *conf);

bool confgenerator_deserialize_mcconf(const uint8_t *buffer, mc_configuration *conf);
bool confgenerator_deserialize_appconf(const uint8_t *buffer, app_configuration *conf);

void disallow_changing_most_mconf_settings(mc_configuration *conf);
void confgenerator_set_defaults_mcconf(mc_configuration *conf, bool all);
void confgenerator_set_defaults_appconf(app_configuration *conf);

// CONFGENERATOR_H_
#endif
