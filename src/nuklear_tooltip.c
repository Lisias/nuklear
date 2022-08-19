#include "nuklear.h"
#include "nuklear_internal.h"

/* ===============================================================
 *
 *                              TOOLTIP
 *
 * ===============================================================*/
NK_API nk_bool
nk_tooltip_begin(struct nk_context *ctx, float width, float height)
{
    int x,y,w,h;
    float sx,sy;
    struct nk_window *win;
    const struct nk_input *in;
    struct nk_rect bounds;
    int ret;
    const struct nk_style *style;
    struct nk_vec2 offset;

    NK_ASSERT(ctx);
    NK_ASSERT(ctx->current);
    NK_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout)
        return 0;

    /* fetch configuration data */
    style = &ctx->style;
    offset = style->window.tooltip_offset;

    /* make sure that no nonblocking popup is currently active */
    win = ctx->current;
    in = &ctx->input;
    if (win->popup.win && (win->popup.type & NK_PANEL_SET_NONBLOCK))
        return 0;

    w = nk_iceilf(width);
    h = nk_iceilf(nk_null_rect.h);
    sx = in->mouse.pos.x + offset.x;
    sy = in->mouse.pos.y + offset.y;
    height = NK_MAX(height, 50.0f);
    if (ctx->popup_screen_bounds.w > 0.0f && ctx->popup_screen_bounds.h > 0.0f) {
        if (sx < ctx->popup_screen_bounds.x) {
            sx = ctx->popup_screen_bounds.x;
        }
        else if ((sx + (float)w) >= (ctx->popup_screen_bounds.x + ctx->popup_screen_bounds.w)) {
            sx = ctx->popup_screen_bounds.x + ctx->popup_screen_bounds.w - (float)w;
        }
        if (sy < ctx->popup_screen_bounds.y) {
            sy = ctx->popup_screen_bounds.y;
        }
        else if ((sy + height) >= (ctx->popup_screen_bounds.y + ctx->popup_screen_bounds.h)) {
            sy = ctx->popup_screen_bounds.y + ctx->popup_screen_bounds.h - height;
        }
    }
    
    x = nk_ifloorf(sx) - (int)win->layout->clip.x;
    y = nk_ifloorf(sy) - (int)win->layout->clip.y;

    bounds.x = (float)x;
    bounds.y = (float)y;
    bounds.w = (float)w;
    bounds.h = (float)h;

    ret = nk_popup_begin(ctx, NK_POPUP_DYNAMIC,
        "__##Tooltip##__", NK_WINDOW_NO_SCROLLBAR|NK_WINDOW_BORDER, bounds);
    if (ret) win->layout->flags &= ~(nk_flags)NK_WINDOW_ROM;
    win->popup.type = NK_PANEL_TOOLTIP;
    ctx->current->layout->type = NK_PANEL_TOOLTIP;
    return ret;
}

NK_API void
nk_tooltip_end(struct nk_context *ctx)
{
    NK_ASSERT(ctx);
    NK_ASSERT(ctx->current);
    if (!ctx || !ctx->current) return;
    ctx->current->seq--;
    nk_popup_close(ctx);
    nk_popup_end(ctx);
}
NK_API void
nk_tooltip(struct nk_context *ctx, const char *text)
{
    const struct nk_style *style;
    struct nk_vec2 padding;

    int text_len;
    float text_width = 0.0f;
    float text_height = 0.0f;
    int glyphs = 0;
    int fitting = 0;
    int rows = 1;
    float width;
    int done = 0;
    NK_INTERN nk_rune seperator[] = {' '};

    NK_ASSERT(ctx);
    NK_ASSERT(ctx->current);
    NK_ASSERT(ctx->current->layout);
    NK_ASSERT(text);
    if (!ctx || !ctx->current || !ctx->current->layout || !text)
        return;

    /* fetch configuration data */
    style = &ctx->style;
    padding = style->window.padding;

    /* calculate size of the text and tooltip */
    text_len = nk_strlen(text);
#if 1
    fitting = nk_text_clamp(style->font, text, text_len, 100000.0f, &glyphs, &width, seperator,NK_LEN(seperator));
    while (done < text_len) {
        if (!fitting) break;
        done += fitting;
        text_width = NK_MAX(width, text_width);
        rows++;
        fitting = nk_text_clamp(style->font, &text[done], text_len - done, 100000.0f, &glyphs, &width, seperator,NK_LEN(seperator));
    }
#else
    text_width = style->font->width(style->font->userdata,
                    style->font->height, text, text_len);
#endif

    text_width += (4 * padding.x);
    text_height = 2 * padding.y + style->font->height;

#if 1
    /* execute tooltip and fill with text */
    if (nk_tooltip_begin(ctx, (float)text_width, text_height*(float)rows)) {
        nk_layout_row_dynamic(ctx, (float)text_height, 1);
        done = 0;
        fitting = nk_text_clamp(style->font, text, text_len, 100000.0f, &glyphs, &width, seperator,NK_LEN(seperator));
        while (done < text_len) {
            if (!fitting) break;
            nk_text(ctx, &text[done], fitting, NK_TEXT_LEFT);
            done += fitting;
            fitting = nk_text_clamp(style->font, &text[done], text_len - done, 100000.0f, &glyphs, &width, seperator,NK_LEN(seperator));
        }
        nk_tooltip_end(ctx);
    }
#else
    /* execute tooltip and fill with text */
    if (nk_tooltip_begin(ctx, (float)text_width)) {
        nk_layout_row_dynamic(ctx, (float)text_height, 1);
        nk_text(ctx, text, text_len, NK_TEXT_CENTERED);
        nk_tooltip_end(ctx);
    }
#endif
}
#ifdef NK_INCLUDE_STANDARD_VARARGS
NK_API void
nk_tooltipf(struct nk_context *ctx, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    nk_tooltipfv(ctx, fmt, args);
    va_end(args);
}
NK_API void
nk_tooltipfv(struct nk_context *ctx, const char *fmt, va_list args)
{
    char buf[256];
    nk_strfmt(buf, NK_LEN(buf), fmt, args);
    nk_tooltip(ctx, buf);
}
#endif


