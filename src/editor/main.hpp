#pragma once

#include <cstring>

#include <GL/glew.h>
#include <GL/gl.h>

#include <SDL3/SDL.h>

#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

#define FONT_SIZE 12

#define INITIAL_WIDTH 800
#define INITIAL_HEIGHT 600

FT_Face face;

extern "C" {
    #include "../../external/microui/src/microui.h"
};

mu_Context ctx;

int width, height;

int text_width(mu_Font font, const char* text, int len) {
    (void)font;

    if (len < 0)
        len = strlen(text);

    int width = 0;

    for (int i = 0; i < len; i++) {
        if (FT_Load_Char(face, (unsigned char)text[i], FT_LOAD_DEFAULT))
            continue;

        width += face->glyph->advance.x >> 6;
    }

    return width;
}

int text_height(mu_Font font) {
    (void)font;
    return face->size->metrics.height >> 6;
}

static unsigned icon_codepoint(int id) {
    switch (id) {
        case MU_ICON_CLOSE:     return 0x2715;  // ✕
        case MU_ICON_CHECK:     return 0x2713;  // ✓
        case MU_ICON_COLLAPSED: return 0x25B6;  // ▶
        case MU_ICON_EXPANDED:  return 0x25BC;  // ▼
        default:                return '?';
    }
}

void draw_icon(int id, mu_Rect rect, mu_Color col) {
    unsigned cp = icon_codepoint(id);
    if (!cp) return;

    if (FT_Load_Char(face, cp, FT_LOAD_RENDER))
        return;

    FT_GlyphSlot g = face->glyph;
    FT_Bitmap* bm = &g->bitmap;
    if (bm->width == 0 || bm->rows == 0) return;

    // Center the *bitmap* in the target rect, not the baseline box.
    int dx = rect.x + (rect.w - (int)bm->width)  / 2;
    int dy = rect.y + (rect.h - (int)bm->rows)   / 2;

    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glColor4ub(col.r, col.g, col.b, col.a);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA8,
                 bm->width, bm->rows, 0,
                 GL_ALPHA, GL_UNSIGNED_BYTE, bm->buffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBegin(GL_QUADS);
        glTexCoord2f(0,0); glVertex2i(dx, dy);
        glTexCoord2f(1,0); glVertex2i(dx + bm->width, dy);
        glTexCoord2f(1,1); glVertex2i(dx + bm->width, dy + bm->rows);
        glTexCoord2f(0,1); glVertex2i(dx, dy + bm->rows);
    glEnd();

    glDeleteTextures(1, &tex);
    glDisable(GL_TEXTURE_2D);
}

void draw_rect(int x, int y, int w, int h, mu_Color col) {
    glColor4ub(col.r, col.g, col.b, col.a);

    glBegin(GL_QUADS);
        glVertex2i(x, y);
        glVertex2i(x+w, y);
        glVertex2i(x+w,y+h);
        glVertex2i(x,y+h);
    glEnd();
}

void draw_text(const char* str, int x, int y, mu_Color col) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glColor4ub(col.r, col.g, col.b, col.a);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (int i = 0; str[i] != '\0'; i++) {
        if (FT_Load_Char(face, str[i], FT_LOAD_RENDER))
            continue;

        FT_GlyphSlot glyph = face->glyph;
        FT_Bitmap* bitmap = &glyph->bitmap;

        int width = bitmap->width;
        int height = bitmap->rows;

        if (width <= 0 || height <= 0) {
            x += glyph->advance.x >> 6;
            continue;
        }

        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_ALPHA8,
            width,
            height,
            0,
            GL_ALPHA,
            GL_UNSIGNED_BYTE,
            bitmap->buffer
        );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        int draw_x = x + glyph->bitmap_left;
        int draw_y = y + (face->size->metrics.ascender >> 6)
                       - glyph->bitmap_top;

        glBegin(GL_QUADS);
            glTexCoord2f(0, 0);
            glVertex2i(draw_x, draw_y);

            glTexCoord2f(1, 0);
            glVertex2i(draw_x + width, draw_y);

            glTexCoord2f(1, 1);
            glVertex2i(draw_x + width, draw_y + height);

            glTexCoord2f(0, 1);
            glVertex2i(draw_x, draw_y + height);
        glEnd();

        glDeleteTextures(1, &texture);

        x += glyph->advance.x >> 6;
    }

    glDisable(GL_TEXTURE_2D);
}

void set_clip(mu_Rect rect) {
    if (rect.w <= 0 || rect.h <= 0) {
        // Empty clip: collapse to a degenerate scissor so nothing draws.
        glScissor(0, 0, 0, 0);
        return;
    }
    int y = height - (rect.y + rect.h);
    glScissor((GLint)rect.x, (GLint)y, (GLsizei)rect.w, (GLsizei)rect.h);
}


void render(mu_Context* ctx) {
    mu_Command* cmd = nullptr;
    glScissor(0, 0, width, height);

    while (mu_next_command(ctx, &cmd)) {
        switch (cmd->type) {
            case MU_COMMAND_RECT:
                draw_rect(
                    cmd->rect.rect.x,
                    cmd->rect.rect.y,
                    cmd->rect.rect.w,
                    cmd->rect.rect.h,
                    cmd->rect.color
                );
                break;

            case MU_COMMAND_TEXT:
                draw_text(
                    cmd->text.str,
                    cmd->text.pos.x,
                    cmd->text.pos.y,
                    cmd->text.color
                );
                break;

            case MU_COMMAND_CLIP: {
                set_clip(cmd->clip.rect);
                break;
            }

            case MU_COMMAND_ICON: {
                draw_icon(cmd->icon.id, cmd->icon.rect, cmd->icon.color);
                break;
            }
        }
    }
}

static void update_viewport(SDL_Window* window) {
    SDL_GetWindowSizeInPixels(window, &width, &height);

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void input_microui(void) {
    float mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);

    mu_input_mousemove(&ctx, (int)mouse_x, (int)mouse_y);
}


void set_colors(void) {
    // Text
    ctx.style->colors[MU_COLOR_TEXT] =
        mu_color(255, 255, 255, 255);

    // Window background
    ctx.style->colors[MU_COLOR_WINDOWBG] =
        mu_color(10, 15, 10, 255);

    // Window title background
    ctx.style->colors[MU_COLOR_TITLEBG] =
        mu_color(0, 5, 0, 255);

    // Buttons
    ctx.style->colors[MU_COLOR_BUTTON] =
        mu_color(50, 200, 100, 255);

    // Button hover
    ctx.style->colors[MU_COLOR_BUTTONHOVER] =
        mu_color(70, 220, 120, 255);

    // Button pressed/focused
    ctx.style->colors[MU_COLOR_BUTTONFOCUS] =
        mu_color(30, 180, 80, 255);

    // Border
    ctx.style->colors[MU_COLOR_BORDER] =
        mu_color(24, 36, 24, 255);
}
