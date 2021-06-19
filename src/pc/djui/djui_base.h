#pragma once
#include "djui.h"

#pragma pack(1)
struct DjuiBaseRect {
    f32 x;
    f32 y;
    f32 width;
    f32 height;
};

#pragma pack(1)
struct DjuiBaseChild {
    struct DjuiBase* base;
    struct DjuiBaseChild* next;
};

#pragma pack(1)
struct DjuiBase {
    struct DjuiBase* parent;
    struct DjuiBaseChild* child;
    bool visible;
    struct DjuiScreenValue x;
    struct DjuiScreenValue y;
    struct DjuiScreenValue width;
    struct DjuiScreenValue height;
    struct DjuiColor color;
    struct DjuiScreenValue borderWidth;
    struct DjuiColor borderColor;
    enum DjuiHAlign hAlign;
    enum DjuiVAlign vAlign;
    struct DjuiBaseRect comp;
    struct DjuiBaseRect clip;
    void (*render)(struct DjuiBase*);
    void (*destroy)(struct DjuiBase*);
};

void djui_base_set_location(struct DjuiBase* base, f32 x, f32 y);
void djui_base_set_location_type(struct DjuiBase* base, enum DjuiScreenValueType xType, enum DjuiScreenValueType yType);
void djui_base_set_size(struct DjuiBase* base, f32 width, f32 height);
void djui_base_set_size_type(struct DjuiBase* base, f32 widthType, f32 heightType);
void djui_base_set_color(struct DjuiBase* base, u8 r, u8 g, u8 b, u8 a);
void djui_base_set_border_width(struct DjuiBase* base, f32 width);
void djui_base_set_border_width_type(struct DjuiBase* base, enum DjuiScreenValueType widthType);
void djui_base_set_border_color(struct DjuiBase* base, u8 r, u8 g, u8 b, u8 a);
void djui_base_set_alignment(struct DjuiBase* base, enum DjuiHAlign hAlign, enum DjuiVAlign vAlign);

void djui_base_compute(struct DjuiBase* base);

void djui_base_render(struct DjuiBase* base);
void djui_base_destroy(struct DjuiBase* base);
void djui_base_init(struct DjuiBase* parent, struct DjuiBase* base, void (*render)(struct DjuiBase*), void (*destroy)(struct DjuiBase*));