#include "zp_cpp/ui.hpp"
#include "zp_cpp/dbg.hpp"

#include <algorithm>
#include <functional>
#include <sstream>
#include <tuple>

using namespace zp::ui;

// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
namespace
{
    struct Unclipped
    {
        ElemIdx elem_idx;
        zp::math::vec4 colour;
        zp::math::vec2 screen_rel_xy;
        zp::math::vec2 screen_rel_wh;
        std::optional<char> text_char;
        std::optional<zp::uuid::uuid> font;
        std::optional<zp::uuid::uuid> bg_img;
        float rot;
        std::optional<std::array<zp::math::vec2, 4>> bezier_cp;
        std::uint32_t bezier_segments;
        float bezier_w;
    };
}

// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
struct Instance::Internal
{
    struct Transient
    {
        std::vector<ElemIdx> root_children;
        std::unordered_map<ElemIdx, std::vector<ElemIdx>> child_idcs;

        std::vector<std::tuple<int, ElemIdx>> comps;

        // need to determine the width and height of all components in a first pass
        // statics have it coded, flexes the size depends on children.
        std::unordered_map<ElemIdx, zp::math::vec2> screen_rel_wh;

        // for parent, where to draw next child that comes up
        std::unordered_map<ElemIdx, zp::math::vec2> resolved_pens;

        // for child, where to be drawn relative to parent
        std::unordered_map<ElemIdx, zp::math::vec2> offsets_to_pars;

        std::unordered_map<ElemIdx, zp::math::bb2> screen_rel_scrollable_area_bbs;

        std::unordered_map<ElemIdx, zp::math::vec2> screen_rel_scroll_offsets;

        std::unordered_map<ElemIdx, zp::math::vec2> screen_rel_cum_scroll_offsets;

        // for child, final rect on screen.
        std::unordered_map<ElemIdx, zp::math::vec2> screen_rel_xy;

        std::vector<Unclipped> unclippeds;

        std::unordered_map<ElemIdx, zp::math::bb2> clip_bbs;

        std::unordered_map<ElemIdx, zp::math::vec4> collisions;
    };

    Transient transient;
};

// =========================================================================================================================================
// =========================================================================================================================================
// init: Allocates the internal bookkeeping needed to process UI elements for the provided instance.
// =========================================================================================================================================
// =========================================================================================================================================
void zp::ui::init(Instance* p_inst)
{
    // ====================================================================================================
    // ====================================================================================================
    // ====================================================================================================
    // ====================================================================================================
    p_inst->p_i = new Instance::Internal();
}

// =========================================================================================================================================
// =========================================================================================================================================
// update: Resolves configured UI elements into drawable quads and text glyphs stored in the output vector.
// =========================================================================================================================================
// =========================================================================================================================================
void zp::ui::update(Instance* p_inst)
{
    auto& t = p_inst->p_i->transient;
    t       = {};

    // ====================================================================================================
    // ====================================================================================================
    // get children
    // ====================================================================================================
    // ====================================================================================================
    {
        for (size_t i = 0; i < p_inst->config.p_elems->count; i++)
        {
            if (p_inst->config.p_elems->p[i].parent_idx.has_value())
            {
                t.child_idcs[p_inst->config.p_elems->p[i].parent_idx.value()].push_back(i);
            }
            else
            {
                t.root_children.push_back(i);
            }
        }
    }

    // ====================================================================================================
    // ====================================================================================================
    // dfs the comps
    // ====================================================================================================
    // ====================================================================================================
    {
        std::function<void(ElemIdx, int)> dfs = [&](ElemIdx node, int depth)
        {
            t.comps.emplace_back(depth, node);
            if (t.child_idcs.contains(node))
            {
                for (auto child : t.child_idcs.at(node)) dfs(child, depth + 1);
            }
        };

        for (auto&& rc : t.root_children)
        {
            dfs(rc, 0);
        }
    }

    // ====================================================================================================
    // ====================================================================================================
    // dev validation
    // ====================================================================================================
    // ====================================================================================================
    {
        for (auto&& [depth, elem_idx] : t.comps)
        {
            const Elem* ptr = p_inst->config.p_elems->p + elem_idx;

            if (ptr->resolve_type == ResolveType::Flex)
            {
                if (t.child_idcs[elem_idx].empty())
                {
                    ERR("flex comp has no children");
                }
            }

            if (ptr->xy_type == XYType::Pc)
            {
                bool invalid_range = ptr->xy.x < -1 || ptr->xy.x > 1 || ptr->xy.y < -1 || ptr->xy.y > 1;
                if (invalid_range) ERR("pcx_pcy invalid range");

                // todo
                // support this
                const Elem* p_par = p_inst->config.p_elems->p + ptr->parent_idx.value();
                bool illegal_use  = p_par->resolve_type == ResolveType::Flex;
                if (illegal_use) ERR("pcx_pcy with a flex parent is unsupported currently");
            }

            if (ptr->wh_type == WHType::Pc)
            {
                bool invalid_range = ptr->wh.x < -1 || ptr->wh.x > 1 || ptr->wh.y < -1 || ptr->wh.y > 1;
                if (invalid_range) ERR("pcw_pch invalid range");

                // todo
                // support this
                const Elem* p_par = p_inst->config.p_elems->p + ptr->parent_idx.value();
                bool illegal_use  = p_par->resolve_type == ResolveType::Flex;
                if (illegal_use) ERR("pcw_pch with a flex parent is unsupported currently");
            }
        }
    }

    // ====================================================================================================
    // ====================================================================================================
    // iterate forwards, settling all statics
    // this is needed rn, as supporting pcw_pch.
    // have to do flexes after this children though, so iterating backwards after
    // this causes an issue when pcw_pch is child of flex, so just disabling that support for now
    // ====================================================================================================
    // ====================================================================================================
    {
        for (auto&& [depth, elem_idx] : t.comps)
        {
            const Elem* ptr = p_inst->config.p_elems->p + elem_idx;

            if (ptr->resolve_type != ResolveType::Static)
            {
                continue;
            }

            if (ptr->wh_type == WHType::Abs)
            {
                t.screen_rel_wh[elem_idx] = ptr->wh;
            }
            else if (ptr->wh_type == WHType::Pc)
            {
                zp::math::vec2 par_w_h    = t.screen_rel_wh[ptr->parent_idx.value()];
                zp::math::vec2 w_h        = ptr->wh * par_w_h;
                t.screen_rel_wh[elem_idx] = w_h;
            }
        }
    }

    // ====================================================================================================
    // ====================================================================================================
    // resolve width and heights of flexes
    // if flex, a component's height and width is determined by its children.
    // note
    // we iterate backwards, as populating the resolved_widths_height array as we go with a child's values,
    // to then use for its parent calculated later in the list.
    // for n=number of children, a flex component's size = padding size + (n-1) * pen spacing + sum of all children's sizes.
    // ^ thats in the direction of the pen. in the other direction, its padding size + size of largest child.
    // ====================================================================================================
    // ====================================================================================================
    {
        for (auto it = t.comps.rbegin(); it != t.comps.rend(); ++it)
        {
            auto&& [depth, elem_idx] = *it;
            const Elem* ptr          = p_inst->config.p_elems->p + elem_idx;

            if (ptr->resolve_type != ResolveType::Flex)
            {
                continue;
            }

            zp::math::vec2 total  = {0, 0};

            // padding on all sides
            total                += zp::math::vec2{2.f * ptr->padding, 2.f * ptr->padding};

            {
                zp::math::vec2 running_largest = {0, 0};
                zp::math::vec2 counting        = {0, 0};

                // gap between children
                {
                    float total_spacing  = ptr->pen_spacing * (t.child_idcs[elem_idx].size() - 1);
                    counting            += zp::math::vec2(total_spacing);
                }

                // size of children
                {
                    for (auto& child_idx : t.child_idcs[elem_idx])
                    {
                        zp::math::vec2 size = t.screen_rel_wh[child_idx];
                        if (size.x > running_largest.x) running_largest.x = size.x;
                        if (size.y > running_largest.y) running_largest.y = size.y;
                        counting += size;
                    }
                }

                if (ptr->pen_dir == PenDir::Vert)
                {
                    total += zp::math::vec2{running_largest.x, counting.y};
                }
                else
                {
                    total += zp::math::vec2{counting.x, running_largest.y};
                }
            }

            t.screen_rel_wh[elem_idx] = total;
        }
    }

    // ====================================================================================================
    // ====================================================================================================
    // resolve offsets to parents
    // tracks and bumps parents' pens
    // ====================================================================================================
    // ====================================================================================================
    {
        for (auto&& [depth, elem_idx] : t.comps)
        {
            const Elem* ptr      = p_inst->config.p_elems->p + elem_idx;
            const Elem* p_parent = ptr->parent_idx.has_value() ? p_inst->config.p_elems->p + ptr->parent_idx.value() : nullptr;

            // init pen with comp's padding
            {
                t.resolved_pens[elem_idx] = zp::math::vec2{ptr->padding, ptr->padding};
            }

            // if comp is absolute pos, no need to interact with parent's resolved pen
            if (ptr->pos_type == PosType::Absolute)
            {
                if (ptr->xy_type == XYType::Abs)
                {
                    t.offsets_to_pars[elem_idx] = ptr->xy;
                }
                else if (ptr->xy_type == XYType::Pc)
                {
                    t.offsets_to_pars[elem_idx] = ptr->xy * t.screen_rel_wh[ptr->parent_idx.value()];
                }
            }
            // if comp is relative pos, use and bump parent's pen
            else if (ptr->pos_type == PosType::Relative)
            {
                zp::math::vec2 par_pen = t.resolved_pens[ptr->parent_idx.value()];

                zp::math::vec2 new_par_pen;
                {
                    new_par_pen = par_pen;
                    if (p_parent->pen_dir == PenDir::Vert)
                    {
                        new_par_pen.y += t.screen_rel_wh[elem_idx].y + p_parent->pen_spacing;
                    }
                    else
                    {
                        new_par_pen.x += t.screen_rel_wh[elem_idx].x + p_parent->pen_spacing;
                    }
                }

                // save results
                t.offsets_to_pars[elem_idx]              = par_pen;
                t.resolved_pens[ptr->parent_idx.value()] = new_par_pen;
            }
        }
    }

    // ====================================================================================================
    // ====================================================================================================
    // use parents' pen offsets with pos data to resolve the rects of all elems
    // ====================================================================================================
    // ====================================================================================================
    {
        for (auto&& [depth, elem_idx] : t.comps)
        {
            const Elem* ptr = p_inst->config.p_elems->p + elem_idx;

            zp::math::vec2 par_xy;
            zp::math::vec2 par_wh;
            {
                if (!ptr->parent_idx.has_value())
                {
                    par_xy = {0, 0};
                    par_wh = {p_inst->config.root_width, p_inst->config.root_height};
                }
                else
                {
                    par_xy = t.screen_rel_xy.at(ptr->parent_idx.value());
                    par_wh = t.screen_rel_wh.at(ptr->parent_idx.value());
                }
            }

            const zp::math::vec2 wh     = t.screen_rel_wh.at(elem_idx);
            const zp::math::vec2 offset = t.offsets_to_pars.at(elem_idx);

            auto& resolved              = t.screen_rel_xy[elem_idx];
            switch (ptr->anchor_type)
            {
                case AnchorType::TopLeft:
                {
                    resolved.x = par_xy.x + offset.x;
                    resolved.y = par_xy.y + offset.y;
                }
                break;
                case AnchorType::TopCenter:
                {
                    resolved.x = par_xy.x + (par_wh.x * 0.5) + offset.x;
                    resolved.y = par_xy.y + offset.y;
                }
                break;
                case AnchorType::TopRight:
                {
                    resolved.x = par_xy.x + par_wh.x - (offset.x + wh.x);
                    resolved.y = par_xy.y + offset.y;
                }
                break;
                case AnchorType::CenterLeft:
                {
                    resolved.x = par_xy.x + offset.x;
                    resolved.y = par_xy.y + (par_wh.y * 0.5) + offset.y;
                }
                break;
                case AnchorType::Center:
                {
                    resolved.x = par_xy.x + (par_wh.x * 0.5) + offset.x;
                    resolved.y = par_xy.y + (par_wh.y * 0.5) + offset.y;
                }
                break;
                case AnchorType::CenterRight:
                {
                    resolved.x = par_xy.x + par_wh.x - (offset.x + wh.x);
                    resolved.y = par_xy.y + (par_wh.y * 0.5) + offset.y;
                }
                break;
                case AnchorType::BottomLeft:
                {
                    resolved.x = par_xy.x + offset.x;
                    resolved.y = par_xy.y + par_wh.y - (offset.y + wh.y);
                }
                break;
                case AnchorType::BottomCenter:
                {
                    resolved.x = par_xy.x + (par_wh.x * 0.5) + offset.x;
                    resolved.y = par_xy.y + par_wh.y - (offset.y + wh.y);
                }
                break;
                case AnchorType::BottomRight:
                {
                    resolved.x = par_xy.x + par_wh.x - (offset.x + wh.x);
                    resolved.y = par_xy.y + par_wh.y - (offset.y + wh.y);
                }
                break;

                default: ERR("invalid anchor type");
            }
        }
    }

    // ====================================================================================================
    // ====================================================================================================
    // calc screen_rel_scrollable_area_bbs
    // ====================================================================================================
    // ====================================================================================================
    {
        for (auto&& [depth, elem_idx] : t.comps)
        {
            const Elem* ptr                            = p_inst->config.p_elems->p + elem_idx;
            const Elem* p_parent                       = ptr->parent_idx.has_value() ? p_inst->config.p_elems->p + ptr->parent_idx.value() : nullptr;

            const zp::math::vec2 xy                    = t.screen_rel_xy.at(elem_idx);
            const zp::math::vec2 wh                    = t.screen_rel_wh.at(elem_idx);

            // note
            // init for self before later iterating onto children.
            t.screen_rel_scrollable_area_bbs[elem_idx] = {.min = xy, .max = xy + wh};

            if (!p_parent)
            {
                continue;
            }

            auto& par_bb = t.screen_rel_scrollable_area_bbs.at(ptr->parent_idx.value());
            if (par_bb.min.x > xy.x) par_bb.min.x = xy.x;
            if (par_bb.min.y > xy.y) par_bb.min.y = xy.y;
            if (par_bb.max.x < xy.x + wh.x) par_bb.max.x = xy.x + wh.x;
            if (par_bb.max.y < xy.y + wh.y) par_bb.max.y = xy.y + wh.y;
        }
    }

    // ====================================================================================================
    // ====================================================================================================
    // calc screen_rel_scroll_offsets
    // ====================================================================================================
    // ====================================================================================================
    {
        for (auto&& [depth, elem_idx] : t.comps)
        {
            const Elem* ptr                       = p_inst->config.p_elems->p + elem_idx;

            zp::math::vec2 scrollable_area_wh     = t.screen_rel_scrollable_area_bbs.at(elem_idx).max - t.screen_rel_scrollable_area_bbs.at(elem_idx).min;

            t.screen_rel_scroll_offsets[elem_idx] = -1 * ptr->scroll_offset * (scrollable_area_wh - t.screen_rel_wh.at(elem_idx));
        }
    }

    // ====================================================================================================
    // ====================================================================================================
    // accumulate scroll offsets
    // ====================================================================================================
    // ====================================================================================================
    for (auto&& [depth, elem_idx] : t.comps)
    {
        const Elem* ptr = p_inst->config.p_elems->p + elem_idx;

        if (ptr->parent_idx.has_value())
        {
            t.screen_rel_cum_scroll_offsets[elem_idx] = t.screen_rel_cum_scroll_offsets.at(ptr->parent_idx.value()) + t.screen_rel_scroll_offsets.at(ptr->parent_idx.value());
        }
        else
        {
            t.screen_rel_cum_scroll_offsets[elem_idx] = {0, 0};
        }
    }

    // ====================================================================================================
    // ====================================================================================================
    // dev
    // going to just overwrite screen_rel_xy right now with the addition of the scroll info, but this is really not idiomatic,
    // it'll be confusing in future why screen_rel_xy produced earlier isnt actually truly the final ver...
    // ====================================================================================================
    // ====================================================================================================
    {
        for (auto&& [depth, elem] : t.comps)
        {
            t.screen_rel_xy[elem] += t.screen_rel_cum_scroll_offsets[elem];
        }
    }

    // ====================================================================================================
    // ====================================================================================================
    // clip_bbs
    // accumulate from ancestors
    // ====================================================================================================
    // ====================================================================================================
    {
        for (auto&& [depth, idx] : t.comps)
        {
            zp::math::bb2 bb = {
                .min = t.screen_rel_xy[idx],
                .max = t.screen_rel_xy[idx] + t.screen_rel_wh[idx],
            };

            if (std::optional<zp::ui::ElemIdx> pidx = p_inst->config.p_elems->p[idx].parent_idx)
            {
                auto& p_bb = t.clip_bbs[pidx.value()];
                bb.min.x   = std::max(bb.min.x, p_bb.min.x);
                bb.min.y   = std::max(bb.min.y, p_bb.min.y);
                bb.max.x   = std::min(bb.max.x, p_bb.max.x);
                bb.max.y   = std::min(bb.max.y, p_bb.max.y);
            }

            t.clip_bbs[idx] = bb;
        }
    }

    // ====================================================================================================
    // ====================================================================================================
    // compile draw infos into renderer friendly data structures
    // ====================================================================================================
    // ====================================================================================================
    {
        for (auto&& [depth, elem_idx] : t.comps)
        {
            const Elem* ptr                    = p_inst->config.p_elems->p + elem_idx;

            const zp::math::vec2 screen_rel_xy = t.screen_rel_xy.at(elem_idx);
            const zp::math::vec2 screen_rel_wh = t.screen_rel_wh.at(elem_idx);

            std::optional<std::string> text    = ptr->text;

            // ===============================================================================================
            // ===============================================================================================
            // for the actual Element Rect
            // ===============================================================================================
            // ===============================================================================================
            {
                Unclipped inst       = {};
                inst.elem_idx        = elem_idx;
                inst.colour          = ptr->bg_col;
                inst.screen_rel_xy   = screen_rel_xy;
                inst.screen_rel_wh   = screen_rel_wh;
                inst.text_char       = std::nullopt;
                inst.bg_img          = ptr->bg_img;
                inst.rot             = ptr->rot;
                inst.bezier_cp       = ptr->bezier_cp;
                inst.bezier_segments = ptr->bezier_segments;
                inst.bezier_w        = ptr->bezier_w;

                t.unclippeds.push_back(inst);
            }

            // ===============================================================================================
            // ===============================================================================================
            // extra n for n characters
            // ===============================================================================================
            // ===============================================================================================
            {
                if (text.has_value())
                {
                    const auto& font  = p_inst->config.p_fonts->at(ptr->font.value());
                    float ascender    = font.ascender * ptr->font_size;
                    float line_height = font.line_height * ptr->font_size;

                    float penX        = screen_rel_xy.x;
                    float penY        = screen_rel_xy.y + ascender;

                    // split text into words
                    std::vector<std::string> words;
                    {
                        std::stringstream ss(text.value());
                        std::string word;
                        while (ss >> word)
                        {
                            words.push_back(word);
                        }
                    }

                    for (auto word : words)
                    {
                        auto chars = std::vector<char>(word.begin(), word.end());
                        chars.push_back(' ');

                        // horizontally wrap words
                        {
                            float estimated_word_width = 0;
                            for (char c : chars)
                            {
                                estimated_word_width += font.glyphs.at(c).advance * ptr->font_size;
                            }
                            float estimated_end_x = penX + estimated_word_width;

                            if (estimated_end_x > screen_rel_xy.x + screen_rel_wh.x)
                            {

                                penX  = screen_rel_xy.x;
                                penY += line_height;
                            }
                        }

                        // add a glyph for each char
                        {
                            for (char c : chars)
                            {
                                const auto& glyph          = font.glyphs.at(c);

                                zp::math::vec2 quad_offset = glyph.quad_offset * ptr->font_size;
                                zp::math::vec2 quad_size   = glyph.quad_size * ptr->font_size;
                                float advance              = glyph.advance * ptr->font_size;
                                zp::math::vec2 quad_pos    = zp::math::vec2{penX + quad_offset.x, penY - quad_offset.y};

                                Unclipped inst             = {
                                                .elem_idx      = elem_idx,
                                                .colour        = ptr->text_col,
                                                .screen_rel_xy = quad_pos,
                                                .screen_rel_wh = quad_size,
                                                .text_char     = c,
                                                .font          = ptr->font.value(),
                                                .bg_img        = std::nullopt,
                                                .rot           = 0,
                                };

                                t.unclippeds.push_back(inst);

                                // get ready for next character
                                penX += advance;
                            }
                        }
                    }
                }
            }
        }
    }

    // ====================================================================================================
    // ====================================================================================================
    // clip
    // ====================================================================================================
    // ====================================================================================================
    {
        p_inst->output.clear();

        for (auto&& unc : t.unclippeds)
        {
            zp::math::vec2 start           = unc.screen_rel_xy;
            zp::math::vec2 end             = unc.screen_rel_xy + unc.screen_rel_wh;
            const zp::math::bb2& clip_bb   = t.clip_bbs.at(unc.elem_idx);

            const zp::math::vec2 root_size = zp::math::vec2{p_inst->config.root_width, p_inst->config.root_height};

            bool is_culled                 = start.x > clip_bb.max.x || start.y > clip_bb.max.y || end.x < clip_bb.min.x || end.y < clip_bb.min.y;

            if (is_culled)
            {
                continue;
            }

            if (!unc.bezier_cp.has_value())
            {
                zp::math::vec2 clip_start = {0, 0};
                zp::math::vec2 clip_end   = {1, 1};
                {
                    if (start.x < clip_bb.min.x)
                    {
                        clip_start.x = (clip_bb.min.x - start.x) / unc.screen_rel_wh.x;
                        start.x      = clip_bb.min.x;
                    }
                    if (start.y < clip_bb.min.y)
                    {
                        clip_end.y = 1 - ((clip_bb.min.y - start.y) / unc.screen_rel_wh.y);
                        start.y    = clip_bb.min.y;
                    }
                    if (end.x > clip_bb.max.x)
                    {
                        clip_end.x = 1 - ((end.x - clip_bb.max.x) / unc.screen_rel_wh.x);
                        end.x      = clip_bb.max.x;
                    }
                    if (end.y > clip_bb.max.y)
                    {
                        clip_start.y = (end.y - clip_bb.max.y) / unc.screen_rel_wh.y;
                        end.y        = clip_bb.max.y;
                    }
                }

                const Resolved resolved = {
                    .elem_idx        = unc.elem_idx,
                    .colour          = unc.colour,
                    .pos             = start / root_size,
                    .scale           = (end - start) / root_size,
                    .clip_start      = clip_start,
                    .clip_end        = clip_end,
                    .text_char       = unc.text_char,
                    .font            = unc.font,
                    .bg_img          = unc.bg_img,
                    .rot             = unc.rot,
                    .bezier_cp       = unc.bezier_cp,
                    .bezier_segments = unc.bezier_segments,
                    .bezier_w        = unc.bezier_w,
                };

                p_inst->output.push_back(std::move(resolved));
            }
            else
            {
                zp::math::vec2 start_nrm     = start / root_size;
                zp::math::vec2 end_nrm       = end / root_size;

                zp::math::rect clip_rect_nrm = {clip_bb.min / root_size, (clip_bb.max - clip_bb.min) / root_size};

                std::array<zp::math::vec2, 4> bezier_cp;
                for (size_t i = 0; i < 4; i++)
                {
                    zp::math::vec2 val  = unc.bezier_cp.value()[i];
                    val                *= (end_nrm - start_nrm);
                    val                += start_nrm;

                    bezier_cp[i]        = val;
                }

                const Resolved resolved = {
                    .elem_idx        = unc.elem_idx,
                    .colour          = unc.colour,
                    .pos             = {0, 0},
                    .scale           = {0, 0},
                    .clip_start      = {0, 0},
                    .clip_end        = {0, 0},
                    .text_char       = unc.text_char,
                    .font            = unc.font,
                    .bg_img          = unc.bg_img,
                    .rot             = unc.rot,
                    .bezier_cp       = bezier_cp,
                    .bezier_segments = unc.bezier_segments,
                    .bezier_w        = unc.bezier_w,
                    .clip_rect_nrm   = clip_rect_nrm,
                };

                p_inst->output.push_back(std::move(resolved));
            }
        }
    }

    // ====================================================================================================
    // ====================================================================================================
    // for collisions
    // ====================================================================================================
    // ====================================================================================================
    {
        for (auto&& r : p_inst->output)
        {
            if (r.text_char.has_value() || r.bezier_cp.has_value())
            {
                continue;
            }

            t.collisions[r.elem_idx] = {r.pos.x, r.pos.y, r.scale.x, r.scale.y};
        }
    }
}

// =========================================================================================================================================
// =========================================================================================================================================
// is_point_inside: Returns true when the provided normalised point lies within the resolved bounds of the element.
// =========================================================================================================================================
// =========================================================================================================================================
bool zp::ui::is_point_inside(zp::ui::Instance* p_inst, ElemIdx elem_idx, zp::math::vec2 nrm_p)
{
    if (!p_inst->p_i->transient.collisions.contains(elem_idx))
    {
        return false;
    }

    zp::math::vec4 xywh = p_inst->p_i->transient.collisions.at(elem_idx);

    return nrm_p.x >= xywh.x && nrm_p.x <= xywh.x + xywh.z && nrm_p.y >= xywh.y && nrm_p.y <= xywh.y + xywh.w;
}

// =========================================================================================================================================
// =========================================================================================================================================
// calc_point_inside: Calculates the relative position of a normalised point inside an element if it intersects.
// =========================================================================================================================================
// =========================================================================================================================================
bool zp::ui::calc_point_inside(Instance* p_inst, ElemIdx elem_idx, zp::math::vec2 nrm_p, zp::math::vec2* p_out)
{
    if (!p_inst->p_i->transient.collisions.contains(elem_idx))
    {
        return false;
    }

    zp::math::vec4 xywh = p_inst->p_i->transient.collisions.at(elem_idx);

    p_out->x            = (nrm_p.x - xywh.x) / xywh.z;
    p_out->y            = (nrm_p.y - xywh.y) / xywh.w;

    return p_out->x >= 0 && p_out->x <= 1 && p_out->y >= 0 && p_out->y <= 1;
}