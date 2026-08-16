#pragma once
//****************************************************************************
// File: EffectsOptions.h
//
// 
// Copyright (c) 2025-2026 DadDesign-Projects.
//****************************************************************************


#if __has_include("EffectsConfig.h")
	#include "EffectsConfig.h"
#else

	#include "EffectsList.h"
		#ifndef ACTIVE_EFFECT
			#define ACTIVE_EFFECT EFFECT_TEMPLATE
		#endif
#endif


// ==========================================================================
// EFFECT INCLUDE DISPATCH
// --------------------------------------------------------------------------
// Includes the header options corresponding to the active effect.
// ==========================================================================
#include "HardwareAndCoDefines.h"

#if ACTIVE_EFFECT == EFFECT_DELAY
    #include "DelaySpecific.h"

#elif ACTIVE_EFFECT == EFFECT_MODULATIONS
    #include "ModulationsSpecific.h"

#elif ACTIVE_EFFECT == EFFECT_REVERB
    #include "ReverbSpecific.h"

#elif ACTIVE_EFFECT == EFFECT_TEMPLATE
    #include "TemplateSpecific.h"

#elif ACTIVE_EFFECT == EFFECT_TEMPLATE_MULTI_MODE
    #include "TemplateMultiSpecific.h"

#elif ACTIVE_EFFECT == EFFECT_IR_LOADER
    #include "IRLoaderSpecific.h"


#else
    #error "ACTIVE_EFFECT is not defined or does not match any known effect."
#endif

//***End of file**************************************************************
