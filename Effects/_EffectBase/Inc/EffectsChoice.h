#pragma once
//****************************************************************************
// File: EffectsChoice.h
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
// Includes the header corresponding to the active effect.
// ==========================================================================

#if ACTIVE_EFFECT == EFFECT_DELAY
    #include "Delay.h"

#elif ACTIVE_EFFECT == EFFECT_MODULATIONS
    #include "cModulations.h"

#elif ACTIVE_EFFECT == EFFECT_REVERB
    #include "Reverb.h"

#elif ACTIVE_EFFECT == EFFECT_TEMPLATE
    #include "cTemplateEffect.h"

#elif ACTIVE_EFFECT == EFFECT_TEMPLATE_MULTI_MODE
    #include "TemplateMultiModeEffect.h"

#elif ACTIVE_EFFECT == EFFECT_IR_LOADER
    #include "cIRLoaderEffect.h"


#else
    #error "ACTIVE_EFFECT is not defined or does not match any known effect."
#endif

//***End of file**************************************************************
