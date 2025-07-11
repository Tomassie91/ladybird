/*
 * Copyright (c) 2022, Sam Atkins <atkinssj@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/ByteString.h>
#include <AK/Optional.h>
#include <LibGfx/Path.h>
#include <LibWeb/HTML/TextMetrics.h>

namespace Web::HTML {
// https://html.spec.whatwg.org/multipage/canvas.html#canvastext
template<typename IncludingClass>
class CanvasText {
public:
    virtual ~CanvasText() = default;

    void fill_text(Utf16String, float x, float y, Optional<double> max_width = 0);
    void stroke_text(Utf16String, float x, float y, Optional<double> max_width = 0);
    GC::Ref<TextMetrics> measure_text(Utf16String const&) = 0;

    struct PreparedText {
        Vector<NonnullRefPtr<Gfx::GlyphRun>> glyph_runs;
        Gfx::TextAlignment physical_alignment;
        Gfx::FloatRect bounding_box;
    };

protected:
    CanvasText() = default;
    virtual Gfx::Path text_path(StringView text, float x, float y, Optional<double> max_width);
    virtual void stroke_internal(Gfx::Path const&);
    virtual void fill_internal(Gfx::Path const&, Gfx::WindingRule);

private:
    static PreparedText prepare_text(ByteString const& text, Optional<double> max_width);
    Variant<CanvasRenderingContext2D, OffscreenCanvasRenderingContext2D> my_realm() { return &reinterpret_cast<IncludingClass&>(*this).realm(); }
    Variant<CanvasRenderingContext2D, OffscreenCanvasRenderingContext2D> my_font_cascade_list() { return &reinterpret_cast<IncludingClass&>(*this).font_cascade_list(); }
};

}
