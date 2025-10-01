#pragma once

#include "zp_cpp/buff.hpp"
#include "zp_cpp/math.hpp"
#include "zp_cpp/uuid.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace zp::ui
{
    using ElemIdx = std::uint32_t;

    enum AnchorType
    {
        TopLeft,
        TopCenter,
        TopRight,
        CenterLeft,
        Center,
        CenterRight,
        BottomLeft,
        BottomCenter,
        BottomRight
    };

    enum PosType
    {
        Absolute,
        Relative
    };

    enum ResolveType
    {
        Static,
        Flex
    };

    enum PenDir
    {
        Vert,
        Horiz,
    };

    enum class XYType
    {
        Abs,
        Pc
    };

    enum class WHType
    {
        Abs,
        Pc,
    };

    struct Elem
    {
        std::optional<ElemIdx> parent_idx                      = std::nullopt;
        PosType pos_type                                       = PosType::Absolute;
        ResolveType resolve_type                               = ResolveType::Static;
        AnchorType anchor_type                                 = AnchorType::TopLeft;
        XYType xy_type                                         = XYType::Abs;
        zp::math::vec2 xy                                      = {0, 0};
        float rot                                              = 0;
        WHType wh_type                                         = WHType::Abs;
        zp::math::vec2 wh                                      = {0, 0};
        float padding                                          = 0.0f;
        PenDir pen_dir                                         = PenDir::Vert;
        float pen_spacing                                      = 0.0f;
        std::optional<std::string> text                        = std::nullopt;
        std::optional<zp::uuid::uuid> font                     = std::nullopt;
        float font_size                                        = 0.f;
        zp::math::vec4 text_col                                = {1.0f, 1.0f, 1.0f, 1.0f};
        std::optional<zp::uuid::uuid> bg_img                   = std::nullopt;
        zp::math::vec4 bg_col                                  = {0.0f, 0.0f, 0.0f, 0.0f};
        zp::math::vec2 scroll_offset                           = {0.0f, 0.0f};
        std::optional<std::array<zp::math::vec2, 4>> bezier_cp = std::nullopt;
        std::uint32_t bezier_segments                          = 0;
        float bezier_w                                         = 0;
    };

    struct FontData
    {
        struct GlyphData
        {
            zp::math::vec2 quad_size;
            zp::math::vec2 quad_offset;
            float advance;
        };

        float line_height;
        float ascender;
        float descender;
        std::unordered_map<unsigned int, GlyphData> glyphs;
    };

    struct Config
    {
        float root_width;
        float root_height;
        std::unordered_map<zp::uuid::uuid, FontData>* p_fonts;
        zp::span<Elem>* p_elems;
    };

    struct Resolved
    {
        ElemIdx elem_idx;
        zp::math::vec4 colour;
        zp::math::vec2 pos;
        zp::math::vec2 scale;
        zp::math::vec2 clip_start;
        zp::math::vec2 clip_end;
        std::optional<char> text_char;
        std::optional<zp::uuid::uuid> font;
        std::optional<zp::uuid::uuid> bg_img;
        float rot;
        std::optional<std::array<zp::math::vec2, 4>> bezier_cp;
        std::uint32_t bezier_segments;
        float bezier_w;
        zp::math::rect clip_rect_nrm;
    };

    struct Instance
    {
        Config config;
        std::vector<Resolved> output;

        struct Internal;
        Internal* p_i;
    };

    // =========================================================================================================================================
    // =========================================================================================================================================
    // init: Allocates the internal bookkeeping needed to process UI elements for the provided instance.
    // =========================================================================================================================================
    // =========================================================================================================================================
    void init(Instance* p_inst);

    // =========================================================================================================================================
    // =========================================================================================================================================
    // update: Resolves configured UI elements into drawable quads and text glyphs stored in the output vector.
    // =========================================================================================================================================
    // =========================================================================================================================================
    void update(Instance* p_inst);

    // =========================================================================================================================================
    // =========================================================================================================================================
    // is_point_inside: Returns true when the provided normalised point lies within the resolved bounds of the element.
    // =========================================================================================================================================
    // =========================================================================================================================================
    bool is_point_inside(Instance* p_inst, ElemIdx elem_idx, zp::math::vec2 nrm_point);

    // =========================================================================================================================================
    // =========================================================================================================================================
    // calc_point_inside: Calculates the relative position of a normalised point inside an element if it intersects.
    // =========================================================================================================================================
    // =========================================================================================================================================
    bool calc_point_inside(Instance* p_inst, ElemIdx elem_idx, zp::math::vec2 nrm_point, zp::math::vec2* p_out);
}
