#pragma once
//****************************************************************************
// File: EffectsConfig.h
//
// 
// Copyright (c) 2025 Dad Design.
//****************************************************************************
#include "ID.h"
#include "stdint.h"
//#define MODULATIONS_EFFECT
//#define DELAY_EFFECT
//#define MODULATIONS_EFFECT
#define TEMPLATE_EFFECT
//#define TEMPLATE_MULTI_MODE_EFFECT

// Configuring the DELAY Template
#ifdef DELAY_EFFECT
#include "Delay.h"
#define DECLARE_EFFECT DadEffect::cDelay __Effect
#define EFFECT_NAME "Delay"
#define EFFECT_VERSION "Version 1.0"
#define EFFECT_SPLATCH_SCREEN "Delay.png"
constexpr uint32_t EFFECT_BUILD =   BUILD_ID('D', 'E', 'L', '1');
#endif

// Configuring the Template effect
#ifdef TEMPLATE_EFFECT
#include "cTemplateEffect.h"
#define DECLARE_EFFECT DadEffect::cTemplateEffect __Effect
#define EFFECT_NAME "Template"
#define EFFECT_VERSION "Version 1.0"
#define EFFECT_SPLATCH_SCREEN "Template.png"
constexpr uint32_t EFFECT_BUILD =   BUILD_ID('T', 'E', 'M', '1');
#endif

// Configuring the Template multi-effect
#ifdef TEMPLATE_MULTI_MODE_EFFECT
#include "TemplateMultiModeEffect.h"
#define DECLARE_EFFECT DadEffect::cTemplateMainMultiModeEffect __Effect
#define EFFECT_NAME "Template multi-mode"
#define EFFECT_VERSION "Version 1.0"
#define EFFECT_SPLATCH_SCREEN "Template.png"
constexpr uint32_t EFFECT_BUILD =   BUILD_ID('T', 'P', 'M', '2');
#endif

// Configuring the Template multi-effect
#ifdef MODULATIONS_EFFECT
#include "cModulations.h"
#define DECLARE_EFFECT DadEffect::cMainModulations __Effect
#define EFFECT_NAME "Modulations"
#define EFFECT_VERSION "Version 1.0"
#define EFFECT_SPLATCH_SCREEN "Modulations.png"
constexpr uint32_t EFFECT_BUILD = BUILD_ID('M', 'O', 'D', '1');
#endif
//***End of file**************************************************************
