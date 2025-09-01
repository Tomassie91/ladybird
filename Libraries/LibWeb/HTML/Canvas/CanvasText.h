/*
 * Copyright (c) 2022, Sam Atkins <atkinssj@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/Debug.h>
#include <AK/Optional.h>
#include <LibGfx/Path.h>
#include <LibWeb/HTML/TextMetrics.h>
#include <LibWeb/HTML/Canvas/CanvasState.h>

namespace Web::HTML {
// https://html.spec.whatwg.org/multipage/canvas.html#canvastext
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
    explicit CanvasText(Bindings::PlatformObject& self, CanvasState const& state, Gfx::FontCascadeList const& font_cascade_list)
        : m_self(self)
        , m_state(state)
        , m_font_cascade_list(font_cascade_list)
    {
    }
    virtual void stroke_internal(Gfx::Path const&) = 0;
    virtual void fill_internal(Gfx::Path const&, Gfx::WindingRule) = 0;

private:
    GC::Ref<Bindings::PlatformObject> m_self;
    CanvasState const& m_state;
    Gfx::FontCascadeList const& m_font_cascade_list;

    Gfx::Path text_path(StringView text, float x, float y, Optional<double> max_width);
    static PreparedText prepare_text(ByteString const& text, Optional<double> max_width);
    CanvasState::DrawingState my_drawing_state() { return m_state.drawing_state(); }
    CanvasState::DrawingState const& my_drawing_state() const { return m_state.drawing_state(); }
    JS::Realm& realm() { return m_self->realm(); }
    Gfx::FontCascadeList const& my_font_cascade_list() const { return m_font_cascade_list; }
};

}
