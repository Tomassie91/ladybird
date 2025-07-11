/*
 * Copyright (c) 2022, Tomas Klinkenberg <ladybird@tomasklinkenberg.nl>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "CanvasText.h"

#include <AK/StringView.h>
#include <LibGfx/Path.h>
#include <LibGfx/TextLayout.h>
#include <LibGfx/WindingRule.h>
#include <LibWeb/Infra/CharacterTypes.h>

namespace Web::HTML {
// https://html.spec.whatwg.org/multipage/canvas.html#dom-context-2d-measuretext
GC::Ref<TextMetrics> CanvasText::measure_text(StringView text)
{
    // The measureText(text) method steps are to run the text preparation
    // algorithm, passing it text and the object implementing the CanvasText
    // interface, and then using the returned inline box return a new
    // TextMetrics object with members behaving as described in the following
    // list:
    auto& realm = my_realm();
    auto prepared_text = prepare_text(text, {}, realm);
    auto metrics = TextMetrics::create(realm);
    // FIXME: Use the font that was used to create the glyphs in prepared_text.
    auto const& font = my_font_cascade_list->first();

    // width attribute: The width of that inline box, in CSS pixels. (The text's advance width.)
    metrics->set_width(prepared_text.bounding_box.width());
    // actualBoundingBoxLeft attribute: The distance parallel to the baseline from the alignment point given by the textAlign attribute to the left side of the bounding rectangle of the given text, in CSS pixels; positive numbers indicating a distance going left from the given alignment point.
    metrics->set_actual_bounding_box_left(-prepared_text.bounding_box.left());
    // actualBoundingBoxRight attribute: The distance parallel to the baseline from the alignment point given by the textAlign attribute to the right side of the bounding rectangle of the given text, in CSS pixels; positive numbers indicating a distance going right from the given alignment point.
    metrics->set_actual_bounding_box_right(prepared_text.bounding_box.right());
    // fontBoundingBoxAscent attribute: The distance from the horizontal line indicated by the textBaseline attribute to the ascent metric of the first available font, in CSS pixels; positive numbers indicating a distance going up from the given baseline.
    metrics->set_font_bounding_box_ascent(font.baseline());
    // fontBoundingBoxDescent attribute: The distance from the horizontal line indicated by the textBaseline attribute to the descent metric of the first available font, in CSS pixels; positive numbers indicating a distance going down from the given baseline.
    metrics->set_font_bounding_box_descent(prepared_text.bounding_box.height() - font.baseline());
    // actualBoundingBoxAscent attribute: The distance from the horizontal line indicated by the textBaseline attribute to the top of the bounding rectangle of the given text, in CSS pixels; positive numbers indicating a distance going up from the given baseline.
    metrics->set_actual_bounding_box_ascent(font.baseline());
    // actualBoundingBoxDescent attribute: The distance from the horizontal line indicated by the textBaseline attribute to the bottom of the bounding rectangle of the given text, in CSS pixels; positive numbers indicating a distance going down from the given baseline.
    metrics->set_actual_bounding_box_descent(prepared_text.bounding_box.height() - font.baseline());
    // emHeightAscent attribute: The distance from the horizontal line indicated by the textBaseline attribute to the highest top of the em squares in the inline box, in CSS pixels; positive numbers indicating that the given baseline is below the top of that em square (so this value will usually be positive). Zero if the given baseline is the top of that em square; half the font size if the given baseline is the middle of that em square.
    metrics->set_em_height_ascent(font.baseline());
    // emHeightDescent attribute: The distance from the horizontal line indicated by the textBaseline attribute to the lowest bottom of the em squares in the inline box, in CSS pixels; positive numbers indicating that the given baseline is above the bottom of that em square. (Zero if the given baseline is the bottom of that em square.)
    metrics->set_em_height_descent(prepared_text.bounding_box.height() - font.baseline());
    // hangingBaseline attribute: The distance from the horizontal line indicated by the textBaseline attribute to the hanging baseline of the inline box, in CSS pixels; positive numbers indicating that the given baseline is below the hanging baseline. (Zero if the given baseline is the hanging baseline.)
    metrics->set_hanging_baseline(font.baseline());
    // alphabeticBaseline attribute: The distance from the horizontal line indicated by the textBaseline attribute to the alphabetic baseline of the inline box, in CSS pixels; positive numbers indicating that the given baseline is below the alphabetic baseline. (Zero if the given baseline is the alphabetic baseline.)
    metrics->set_font_bounding_box_ascent(0);
    // ideographicBaseline attribute: The distance from the horizontal line indicated by the textBaseline attribute to the ideographic-under baseline of the inline box, in CSS pixels; positive numbers indicating that the given baseline is below the ideographic-under baseline. (Zero if the given baseline is the ideographic-under baseline.)
    metrics->set_font_bounding_box_ascent(0);

    return metrics;
}

void CanvasRenderingContext2D::fill_text(Utf16String const& text, float x, float y, Optional<double> max_width)
{
    fill_internal(text_path(text, x, y, max_width), Gfx::WindingRule::Nonzero);
}

void CanvasRenderingContext2D::stroke_text(Utf16String const& text, float x, float y, Optional<double> max_width)
{
    stroke_internal(text_path(text, x, y, max_width));
}

// https://html.spec.whatwg.org/multipage/canvas.html#text-preparation-algorithm
CanvasText::PreparedText CanvasText::prepare_text(ByteString const& text, Optional<double> max_width)
{
    // 1. If maxWidth was provided but is less than or equal to zero or equal to NaN, then return an empty array.
    if (max_width.has_value() && max_width.value() <= 0) {
        return {};
    }

    // 2. Replace all ASCII whitespace in text with U+0020 SPACE characters.
    StringBuilder builder { text.length() };
    for (auto c : text) {
        builder.append(Infra::is_ascii_whitespace(c) ? ' ' : c);
    }
    auto replaced_text = MUST(builder.to_string());

    // 3. Let font be the current font of target, as given by that object's font attribute.
    auto glyph_runs = Gfx::shape_text({ 0, 0 }, Utf8View(replaced_text), *font_cascade_list());

    // FIXME: 4. Let language be the target's language.
    // FIXME: 5. If language is "inherit":
    //           ...
    // FIXME: 6. If language is the empty string, then set language to explicitly unknown.

    // FIXME: 7. Apply the appropriate step from the following list to determine the value of direction:
    //           ...

    // 8. Form a hypothetical infinitely-wide CSS line box containing a single inline box containing the text text,
    //    with the CSS content language set to language, and with its CSS properties set as follows:
    //   'direction'         -> direction
    //   'font'              -> font
    //   'font-kerning'      -> target's fontKerning
    //   'font-stretch'      -> target's fontStretch
    //   'font-variant-caps' -> target's fontVariantCaps
    //   'letter-spacing'    -> target's letterSpacing
    //   SVG text-rendering  -> target's textRendering
    //   'white-space'       -> 'pre'
    //   'word-spacing'      -> target's wordSpacing
    // ...and with all other properties set to their initial values.
    // FIXME: Actually use a LineBox here instead of, you know, using the default font and measuring its size (which is not the spec at all).
    // FIXME: Once we have CanvasTextDrawingStyles, add the CSS attributes.
    float height = 0;
    float width = 0;
    for (auto const& glyph_run : glyph_runs) {
        height = max(height, glyph_run->font().pixel_size());
        width += glyph_run->width();
    }

    // 9. If maxWidth was provided and the hypothetical width of the inline box in the hypothetical line box is greater than maxWidth CSS pixels, then change font to have a more condensed font (if one is available or if a reasonably readable one can be synthesized by applying a horizontal scale factor to the font) or a smaller font, and return to the previous step.
    // FIXME: Record the font size used for this piece of text, and actually retry with a smaller size if needed.

    // FIXME: 10. The anchor point is a point on the inline box, and the physical alignment is one of the values left, right,
    //            and center. These variables are determined by the textAlign and textBaseline values as follows:
    //            ...

    // 11. Let result be an array constructed by iterating over each glyph in the inline box from left to right (if
    //     any), adding to the array, for each glyph, the shape of the glyph as it is in the inline box, positioned on
    //     a coordinate space using CSS pixels with its origin is at the anchor point.
    CanvasText::PreparedText prepared_text { move(glyph_runs), Gfx::TextAlignment::CenterLeft, { 0, 0, width, height } };

    // 12. Return result, physical alignment, and the inline box.
    return prepared_text;
}
}
