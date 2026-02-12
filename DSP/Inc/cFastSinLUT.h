//==================================================================================
//==================================================================================
// File: cFastSinLUT.h
// Description: LUT Look-Up Table fast sin implementation
//
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#pragma once
#include "main.h"
#include <cmath>
#include <array>

namespace DadDSP {

//**********************************************************************************
// cFastSin
//
//**********************************************************************************
template <size_t LUT_SIZE = 256>
class cFastSinLUT {
private:
    // Table de pré-calcul des valeurs de sinus
    std::array<float, LUT_SIZE> m_lut;

public:
    // Constructeur : initialise la table une fois pour toutes
    cFastSinLUT() {
        // Pré-calcul des valeurs de sinus sur [0, 2π[
        for (size_t i = 0; i < LUT_SIZE; ++i) {
            m_lut[i] = std::sin(2.0f * M_PI * i / static_cast<float>(LUT_SIZE));
        }
    }

    // Fonction principale : calcule sin(angle) rapidement
    inline float operator()(float angle) const {
        // 1. Ramener l'angle dans [0, 2π[
        angle = std::fmod(angle, 2.0f * M_PI);
        if (angle < 0) angle += 2.0f * M_PI;

        // 2. Convertir l'angle en index dans la table
        // Division remplacée par multiplication pour la performance
        constexpr float index_scale = LUT_SIZE / (2.0f * M_PI);
        size_t index = static_cast<size_t>(angle * index_scale);

        // 3. Retourner la valeur pré-calculée (reste de la division pour sécurité)
        return m_lut[index % LUT_SIZE];
    }

    // Accesseur direct à la table (pour débogage ou usage avancé)
    inline const std::array<float, LUT_SIZE>& getTable() const {
        return m_lut;
    }
};// class cFastSin

} // namespace DadDSP
//***End of file**************************************************************
