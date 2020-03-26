// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Text/FontGlyphIDs.hpp"
#include "TTauri/Text/AttributedGrapheme.hpp"
#include "TTauri/Text/GlyphMetrics.hpp"
#include "TTauri/Foundation/mat.hpp"

namespace TTauri::Text {

/**
*/
struct AttributedGlyph {
    FontGlyphIDs glyphs;

    Grapheme grapheme;

    /** Copied from the original attributed-grapheme.
    * An attributed-glyph always represents one or more (ligature) graphemes, a grapheme is never split.
    */
    ssize_t index;

    /** Number of graphemes merged (ligature) into this attributed-glyph. */
    uint8_t grapheme_count;

    /** Copied from the original attributed-grapheme. */
    TextStyle style;

    /** Metrics taken from the font file. */
    GlyphMetrics metrics;

    /** Transform includes position, and scale of the glyph.
     */
    mat transform;

    AttributedGlyph(AttributedGrapheme const &attr_grapheme, FontGlyphIDs glyphs) noexcept :
        glyphs(std::move(glyphs)), grapheme(attr_grapheme.grapheme), index(attr_grapheme.index), grapheme_count(1), style(attr_grapheme.style), metrics() {}

    AttributedGlyph(AttributedGlyph const &other) = default;
    AttributedGlyph(AttributedGlyph &&other) noexcept = default;
    AttributedGlyph &operator=(AttributedGlyph const &other) = default;
    AttributedGlyph &operator=(AttributedGlyph &&other) noexcept = default;
    ~AttributedGlyph() = default;

    [[nodiscard]] Path get_path() const noexcept;
};

}